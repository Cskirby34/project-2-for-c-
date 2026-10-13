int load_valid_words(valid_word_list_t *valid_words) {
  FILE *file = fopen(WORD_LIST_FILENAME, "r");
  if (file == NULL) {
    return 0;
  }

  valid_words->num_words = 0;
  char word[WORD_SIZE + 2];

  while (valid_words->num_words < MAX_VALID_WORDS &&
         fscanf(file, "%6s", word) == 1) {
    if (strlen(word) != WORD_SIZE) {
      continue;
    }
    for (int i = 0; i < WORD_SIZE; i++) {
      word[i] = (char)tolower((unsigned char)word[i]);
    }
    strcpy(valid_words->words[valid_words->num_words], word);
    valid_words->num_words++;
  }

  int ok = !ferror(file);
  fclose(file);
  return ok;
}

int load_todays_answer(char answer[]) {
  FILE *file = fopen(TODAYS_ANSWER_FILENAME, "r");
  if (file == NULL) {
    return 0;
  }

  char word[WORD_SIZE + 2];
  int found = (fscanf(file, "%6s", word) == 1) && strlen(word) == WORD_SIZE;
  fclose(file);
  if (!found) {
    return 0;
  }

  for (int i = 0; i < WORD_SIZE; i++) {
    answer[i] = (char)tolower((unsigned char)word[i]);
  }
  answer[WORD_SIZE] = '\0';
  return 1;
}

void load_stats(player_stats_t *stats) {
  for (int i = 0; i < MAX_NUM_ATTEMPTS; i++) {
    stats->wins_per_num_attempts[i] = 0;
  }
  stats->num_missed_words = 0;

  FILE *file = fopen(STATS_FILENAME, "r");
  if (file == NULL) {
    return;
  }

  player_stats_t tmp;
  int ok = 1;
  for (int i = 0; i < MAX_NUM_ATTEMPTS && ok; i++) {
    ok = fscanf(file, "%u", &tmp.wins_per_num_attempts[i]) == 1;
  }
  if (ok) {
    ok = fscanf(file, "%u", &tmp.num_missed_words) == 1;
  }
  fclose(file);

  if (ok) {            /* only keep the data if the whole file parsed */
    *stats = tmp;
  }
}

int read_attempt(unsigned int num_attempt, char attempt[]) {
  printf("Attempt #%u: ", num_attempt);
  fflush(stdout);

  /* leading whitespace skipped, stops at whitespace, max 5 chars */
  if (scanf(" %5s", attempt) != 1) {
    return 0;
  }
  for (int i = 0; attempt[i] != '\0'; i++) {
    attempt[i] = (char)tolower((unsigned char)attempt[i]);
  }
  return 1;
}

int attempt_is_valid(const valid_word_list_t *valid_words, const char attempt[]) {
  for (unsigned int i = 0; i < valid_words->num_words; i++) {
    if (strcmp(valid_words->words[i], attempt) == 0) {
      return 1;
    }
  }
  fprintf(stderr, "'%s' is not a valid word.\n", attempt);
  return 0;
}

int compare_result(const char todays_answer[], const char attempt[], letter_result_t result[]) {
  int used[WORD_SIZE] = {0};   /* answer letters already matched */
  int all_correct = 1;

  /* Pass 1: exact matches */
  for (int i = 0; i < WORD_SIZE; i++) {
    if (attempt[i] == todays_answer[i]) {
      result[i] = LR_IN_PLACE;
      used[i] = 1;
    } else {
      result[i] = LR_INCORRECT;
      all_correct = 0;
    }
  }

  /* Pass 2: right letter, wrong place, consuming unused answer letters */
  for (int i = 0; i < WORD_SIZE; i++) {
    if (result[i] == LR_IN_PLACE) continue;
    for (int j = 0; j < WORD_SIZE; j++) {
      if (!used[j] && attempt[i] == todays_answer[j]) {
        result[i] = LR_WRONG_PLACE;
        used[j] = 1;
        break;
      }
    }
  }

  return all_correct;
}

void print_attempt_result(const char attempt[], const letter_result_t result[]) {
  printf("Result: ");
  for (int i = 0; i < WORD_SIZE; i++) {
    switch (result[i]) {
      case LR_IN_PLACE:    printf(LETTER_FORMAT_IN_PLACE, attempt[i]);    break;
      case LR_WRONG_PLACE: printf(LETTER_FORMAT_WRONG_PLACE, attempt[i]); break;
      default:             printf(LETTER_FORMAT_INCORRECT, attempt[i]);   break;
    }
  }
  printf("\n");
}

int save_stats(player_stats_t *stats, unsigned int num_attempts) {
  if (num_attempts >= 1 && num_attempts <= MAX_NUM_ATTEMPTS) {
    stats->wins_per_num_attempts[num_attempts - 1]++;
  } else {
    stats->num_missed_words++;
  }

  FILE *file = fopen(STATS_FILENAME, "w");
  if (file == NULL) {
    return 0;
  }

  int ok = 1;
  for (int i = 0; i < MAX_NUM_ATTEMPTS; i++) {
    if (fprintf(file, "%u ", stats->wins_per_num_attempts[i]) < 0) ok = 0;
  }
  if (fprintf(file, "%u\n", stats->num_missed_words) < 0) ok = 0;

  if (fclose(file) != 0) ok = 0;
  return ok;
}

void print_stats(const player_stats_t *stats) {
  unsigned int wins = 0;
  for (int i = 0; i < MAX_NUM_ATTEMPTS; i++) {
    wins += stats->wins_per_num_attempts[i];
  }
  unsigned int played = wins + stats->num_missed_words;
  double win_pct = played == 0 ? 0.0 : 100.0 * wins / played;

  printf("Played: %u\n", played);
  printf("Win %%: %.1f%%\n\n", win_pct);
  printf("Guess distribution:\n");

  for (int i = 0; i < MAX_NUM_ATTEMPTS; i++) {
    unsigned int n = stats->wins_per_num_attempts[i];
    printf("%d: ", i + 1);
    if (n > 0) {
      for (unsigned int s = 0; s < n; s++) putchar('*');
      putchar(' ');
    }
    printf("%u\n", n);
  }
}
