#include "../../include/bricllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

int levenshtein_distance(const char *str1, const char *str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    int *prev_row = malloc((len1 + 1) * sizeof(int));
    int *curr_row = malloc((len1 + 1) * sizeof(int));
    
    if (!prev_row || !curr_row) {
        free(prev_row);
        free(curr_row);
        return len1 > len2 ? len1 : len2;
    }

    for (int i = 0; i <= len1; i++) {
        prev_row[i] = i;
    }

    for (int j = 1; j <= len2; j++) {
        curr_row[0] = j;
        
        for (int i = 1; i <= len1; i++) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            
            int delete_cost = prev_row[i] + 1;
            int insert_cost = curr_row[i - 1] + 1;
            int subst_cost = prev_row[i - 1] + cost;
            
            curr_row[i] = delete_cost;
            if (insert_cost < curr_row[i]) curr_row[i] = insert_cost;
            if (subst_cost < curr_row[i]) curr_row[i] = subst_cost;
        }
        
        int *temp = prev_row;
        prev_row = curr_row;
        curr_row = temp;
    }

    int result = prev_row[len1];
    
    free(prev_row);
    free(curr_row);

    return result;
}

float calculate_similarity(const char *str1, const char *str2) {
    if (!str1 || !str2) return 0.0f;
    if (strlen(str1) == 0 || strlen(str2) == 0) return 0.0f;

    char *str1_lower = strdup(str1);
    char *str2_lower = strdup(str2);

    for (int i = 0; str1_lower[i]; i++) {
        str1_lower[i] = tolower(str1_lower[i]);
    }
    for (int i = 0; str2_lower[i]; i++) {
        str2_lower[i] = tolower(str2_lower[i]);
    }

    int distance = levenshtein_distance(str1_lower, str2_lower);
    int max_len = fmax(strlen(str1_lower), strlen(str2_lower));

    free(str1_lower);
    free(str2_lower);

    if (max_len == 0) return 1.0f;

    return 1.0f - ((float)distance / max_len);
}

char **extract_words(const char *text, int *word_count) {
    if (!text || strlen(text) == 0) {
        *word_count = 0;
        return NULL;
    }

    int capacity = 16;
    char **words = malloc(capacity * sizeof(char *));
    if (!words) {
        *word_count = 0;
        return NULL;
    }
    
    *word_count = 0;
    
    char *text_copy = strdup(text);
    if (!text_copy) {
        free(words);
        *word_count = 0;
        return NULL;
    }
    
    char *token = strtok(text_copy, " \t\n\r");
    
    while (token != NULL) {
        if (*word_count >= capacity) {
            capacity *= 2;
            char **new_words = realloc(words, capacity * sizeof(char *));
            if (!new_words) {
                for (int i = 0; i < *word_count; i++) {
                    free(words[i]);
                }
                free(words);
                free(text_copy);
                *word_count = 0;
                return NULL;
            }
            words = new_words;
        }
        
        int len = strlen(token);
        words[*word_count] = malloc((len + 1) * sizeof(char));
        if (!words[*word_count]) {
            for (int i = 0; i < *word_count; i++) {
                free(words[i]);
            }
            free(words);
            free(text_copy);
            *word_count = 0;
            return NULL;
        }
        
        for (int j = 0; j <= len; j++) {
            words[*word_count][j] = tolower(token[j]);
        }
        
        (*word_count)++;
        token = strtok(NULL, " \t\n\r");
    }

    free(text_copy);
    
    if (*word_count == 0) {
        free(words);
        return NULL;
    }
    
    return words;
}

static ResponsePattern *create_simple_pattern(const char *response, const char *category, float score) {
    ResponsePattern *pattern = malloc(sizeof(ResponsePattern));
    if (!pattern) return NULL;
    
    pattern->response = strdup(response);
    pattern->category = strdup(category);
    
    if (!pattern->response || !pattern->category) {
        free(pattern->response);
        free(pattern->category);
        free(pattern);
        return NULL;
    }
    
    pattern->confidence_threshold = score;
    pattern->keywords = NULL;
    pattern->keyword_count = 0;
    pattern->user_role = NULL;
    pattern->language = NULL;
    
    return pattern;
}

ResponsePattern *find_matching_pattern(const char *message, const char *role, const char *language) {
    if (!message || !role || !language) {
        return NULL;
    }

    log_message("INFO", "Searching for pattern: role=%s, lang=%s, message=%.50s",
                role, language, message);

    int word_count;
    char **message_words = extract_words(message, &word_count);

    if (!message_words || word_count == 0) {
        return NULL;
    }

    ResponsePattern *best_match = NULL;
    float best_score = 0.0f;

    static const char *identity_responses[] = {
        "I'm an assistant designed to help you use the Briconomy app effectively. How can I help you today?",
        "I'm here to guide you through the app. What would you like to know?",
        "I focus on helping with Briconomy features. What can I assist you with?"
    };
    
    static const char *greetings[] = {
        "Hello! I'm here to help you navigate the Briconomy app. What can I assist you with today?",
        "Hi there! How can I help you with the app today?",
        "Hey! What would you like to know about Briconomy?"
    };
    
    static const char *offtopic_responses[] = {
        "I focus on helping with the Briconomy app. Is there something specific about the app I can help you with?",
        "I'm here to assist with app features. What would you like to know about Briconomy?",
        "My specialty is the Briconomy app. How can I help you navigate it?"
    };

    for (int i = 0; i < word_count; i++) {
        if (strcmp(message_words[i], "human") == 0 ||
            strcmp(message_words[i], "robot") == 0 ||
            strcmp(message_words[i], "bot") == 0 ||
            strcmp(message_words[i], "ai") == 0 ||
            strcmp(message_words[i], "real") == 0) {

            float score = 0.92f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern(identity_responses[rand() % 3], "identity", 0.8f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "hello") == 0 ||
            strcmp(message_words[i], "hi") == 0 ||
            strcmp(message_words[i], "hey") == 0 ||
            strcmp(message_words[i], "greetings") == 0) {

            float score = 0.95f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern(greetings[rand() % 3], "greeting", 0.9f);
                if (best_match) best_score = score;
            }
        }

        if ((strcmp(message_words[i], "doing") == 0 || strcmp(message_words[i], "feel") == 0 || strcmp(message_words[i], "feeling") == 0) && word_count < 6) {
            int has_bot_words = 0;
            for (int j = 0; j < word_count; j++) {
                if (strcmp(message_words[j], "bot") == 0 || strcmp(message_words[j], "robot") == 0 || 
                    strcmp(message_words[j], "ai") == 0 || strcmp(message_words[j], "human") == 0) {
                    has_bot_words = 1;
                    break;
                }
            }
            if (has_bot_words) continue;

            float score = 0.88f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern("I'm functioning well, thanks for asking! How can I help you with the Briconomy app?", "smalltalk", 0.75f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "weather") == 0 ||
            strcmp(message_words[i], "day") == 0 ||
            strcmp(message_words[i], "today") == 0 ||
            strcmp(message_words[i], "time") == 0 ||
            strcmp(message_words[i], "date") == 0) {

            float score = 0.85f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern(offtopic_responses[rand() % 3], "offtopic", 0.7f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "rent") == 0 ||
            strcmp(message_words[i], "pay") == 0 ||
            strcmp(message_words[i], "payment") == 0) {

            float score = 0.9f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern("You can manage your rent payments through the Payments section. Check your payment history and make payments using your preferred method. Available buttons: [Home, Payments, Requests, Profile]", "payment", 0.8f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "thanks") == 0 ||
            strcmp(message_words[i], "thank") == 0 ||
            strcmp(message_words[i], "thankyou") == 0) {

            float score = 0.95f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern("You're welcome! Let me know if you need anything else.", "thanks", 0.9f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "joke") == 0 ||
            strcmp(message_words[i], "story") == 0 ||
            strcmp(message_words[i], "game") == 0) {

            float score = 0.88f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern("I'm here to help with the Briconomy app. What would you like to know about managing your property?", "entertainment", 0.8f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "maintenance") == 0 ||
            strcmp(message_words[i], "repair") == 0 ||
            strcmp(message_words[i], "broken") == 0) {

            float score = 0.9f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                best_match = create_simple_pattern("You can report maintenance issues through the Requests section. Include photos and describe the problem for faster resolution. Available buttons: [Home, Payments, Requests, Profile]", "maintenance", 0.8f);
                if (best_match) best_score = score;
            }
        }

        if (strcmp(message_words[i], "where") == 0 ||
            strcmp(message_words[i], "find") == 0 ||
            strcmp(message_words[i], "navigate") == 0 ||
            strcmp(message_words[i], "how") == 0) {

            float score = 0.8f;
            if (score > best_score) {
                if (best_match) {
                    free(best_match->response);
                    free(best_match->category);
                    free(best_match);
                }
                
                const char *nav_response;
                if (strcmp(role, "tenant") == 0) {
                    nav_response = "As a tenant, your main navigation buttons are: [Home, Payments, Requests, Profile]. What would you like to do?";
                } else if (strcmp(role, "caretaker") == 0) {
                    nav_response = "As a caretaker, your navigation buttons are: [Tasks, Schedule, History, Profile]. What would you like to do?";
                } else if (strcmp(role, "manager") == 0) {
                    nav_response = "As a manager, your navigation buttons are: [Dashboard, Properties, Leases, Payments]. What would you like to do?";
                } else {
                    nav_response = "I can help you navigate the Briconomy app. What are you looking for?";
                }
                
                best_match = create_simple_pattern(nav_response, "navigation", 0.7f);
                if (best_match) best_score = score;
            }
        }
    }

    for (int i = 0; i < word_count; i++) {
        free(message_words[i]);
    }
    free(message_words);

    if (best_match) {
        log_message("INFO", "Found pattern match: category=%s, score=%.2f",
                    best_match->category, best_score);
    } else {
        log_message("INFO", "No pattern match found");
    }

    return best_match;
}

void free_pattern(ResponsePattern *pattern) {
    if (!pattern) return;

    if (pattern->keywords) {
        for (int i = 0; i < pattern->keyword_count; i++) {
            free(pattern->keywords[i]);
        }
        free(pattern->keywords);
    }

    free(pattern->response);
    free(pattern->category);
    free(pattern->user_role);
    free(pattern->language);
    free(pattern);
}