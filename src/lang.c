/*
 * IceBreaker - Nintendo 3DS port
 * Copyright (c) 2026 Jean-Baptiste Langlois
 *
 * This file is part of IceBreaker.
 *
 * <https://github.com/Mithweth/icebreaker-3ds>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "lang.h"

#define LANGUAGE_DIRECTORY "romfs:/lang"
#define MAX_LANGUAGES 16
#define MAX_TRANSLATIONS 256
#define MAX_LINE_LENGTH  128


typedef struct {
	char *key;
	char *value;
} Translation;

typedef struct {
	char filename[270];
	int order;
	char name[32];
} Language;

static Language languages[MAX_LANGUAGES];
static size_t language_count;
static size_t current_language;
static size_t selected_language;
static Translation translations[MAX_TRANSLATIONS];
static size_t translation_count = 0;

static int compare_languages(const void *a, const void *b) {
	const Language *lang_a = a;
	const Language *lang_b = b;
	return lang_a->order - lang_b->order;
}

void lang_close(void) {
	for (size_t i = 0; i < translation_count; i++) {
		free(translations[i].key);
		free(translations[i].value);
	}

	translation_count = 0;
}

// Loads the language at current_language, replacing the current translations.
// If the file can't be opened, the previous translations stay loaded.
static bool lang_load(void) {
	Language *lang = &languages[current_language];
	FILE *file = fopen(lang->filename, "r");

	if (!file) {
		fprintf(stderr, "Can't open language file %s\n", lang->filename);
		return false;
	}

	lang_close();

	char line[MAX_LINE_LENGTH];

	while (fgets(line, sizeof(line), file)) {

		line[strcspn(line, "\r\n")] = '\0';
		if (line[0] == '\0' || line[0] == '#') {
			continue;
		}

		char *separator = strchr(line, '=');

		if (!separator) {
			continue;
		}

		*separator = '\0';

		char *key = line;
		char *value = separator + 1;

		if (*key == '\0') {
			continue;
		}

		if (translation_count >= MAX_TRANSLATIONS) {
			break;
		}

		translations[translation_count].key = strdup(key);
		translations[translation_count].value = strdup(value);

		if (!translations[translation_count].key || !translations[translation_count].value) {
			free(translations[translation_count].key);
			free(translations[translation_count].value);
			fclose(file);
			lang_close();
			return false;
		}
		translation_count++;
	}
	printf("language loaded successfully: %s\n", lang->name);
	printf("%s: Loaded %zu translations\n", lang->filename, translation_count);
	fclose(file);
	return true;
}

void lang_next(void) {
	selected_language++;
	if (selected_language >= language_count) {
		selected_language = 0;
	}
}

void lang_prev(void) {
	if (selected_language == 0) {
		selected_language = language_count - 1;
	} else {
		selected_language--;
	}
}

const char* lang_selected_name(void) {
	return languages[selected_language].name;
}

const char* lang_current_name(void) {
	return languages[current_language].name;
}

void lang_apply_selected(void) {
	current_language = selected_language;
	if (!lang_load()) {
		printf("Error loading language: %s\n", languages[current_language].filename);
	}
}

bool lang_set_current_name(const char *lang) {
	for(int i = 0; i < language_count; i++) {
		if (strcmp(languages[i].name, lang) == 0) {
			current_language = i;
			if (!lang_load()) {
				printf("Error loading language: %s\n", languages[current_language].filename);
				return false;
			}
			return true;
		}
	}
	return false;
}

void lang_selected_reset(void) {
	selected_language = current_language;
}

bool lang_init(void) {
	struct dirent *entry;
	Language *lang = NULL;
	DIR *dir = opendir(LANGUAGE_DIRECTORY);
	if (!dir) {
		return false;
	}
	while ((entry = readdir(dir))) {
		char line[MAX_LINE_LENGTH];
		char fullname[270];
		snprintf(fullname, sizeof(fullname), "%s/%s", LANGUAGE_DIRECTORY, entry->d_name);
		FILE *file = fopen(fullname, "r");
		if (!file) {
			continue;
		}
		if (language_count >= MAX_LANGUAGES) {
			printf("Too many languages in %s (max %d)\n", LANGUAGE_DIRECTORY, MAX_LANGUAGES);
			fclose(file);
			break;
		}
		printf("Browsing lang file %s\n", entry->d_name);
		lang = &languages[language_count++];
		memset(lang, 0, sizeof(*lang));
		snprintf(lang->filename, sizeof(lang->filename), "%s", fullname);

		while (fgets(line, sizeof(line), file)) {
			line[strcspn(line, "\r\n")] = '\0';
			if (line[0] == '\0' || line[0] == '#') {
				continue;
			}
			char *separator = strchr(line, '=');
			if (!separator) {
				continue;
			}
			*separator = '\0';
			char *key = line;
			char *value = separator + 1;

			if (*key == '\0') {
				continue;
			}
			if (strcmp(key, "ORDER") == 0) {
				lang->order = atoi(value);
			}
			if (strcmp(key, "NAME") == 0) {
				snprintf(lang->name, sizeof(lang->name), "%s", value);
			}
		}
		lang = NULL;
		fclose(file);
	}
	qsort(languages, language_count, sizeof(Language), compare_languages);
	current_language = 0;
	closedir(dir);
	return true;
}

const char *lang_get(const char *key) {
	for (size_t i = 0; i < translation_count; i++) {
		if (strcmp(translations[i].key, key) == 0)
			return translations[i].value;
	}
// Missing keys fall back to the key itself, so untranslated text shows up on
// screen instead of crashing.
	return key;
}
