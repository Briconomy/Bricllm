#include "include/bricllm.h"
#include "include/chat_engine.h"
#include "include/route_types.h"
#include "include/pattern_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

static void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --single-query <message>, -q <message>    Process one query and exit\n");
    printf("  --role <role>                             Set user role (tenant, caretaker, manager, admin)\n");
    printf("  --lang <lang>                             Set language (en, zu)\n");
    printf("  --route <path>                            Set current route context\n");
    printf("  --json-output, -j                         Output responses as JSON\n");
    printf("  --help, -h                                Show this help message\n");
}

void show_help(void) {
    printf("=== Bricllm Commands ===\n");
    printf("/help                 - Show this help message\n");
    printf("/clear                - Clear screen\n");
    printf("/role <role>          - Set user role (tenant, caretaker, manager, admin)\n");
    printf("/lang <lang>          - Set language (en, zu)\n");
    printf("/route <path>         - Set current route context\n");
    printf("/status               - Show current session status\n");
    printf("/stats                - Show cache statistics\n");
    printf("/quit                 - Exit the application\n");
    printf("\n");
    printf("Natural language examples:\n");
    printf("- \"How do I pay my rent?\"\n");
    printf("- \"Where can I find maintenance requests?\"\n");
    printf("- \"Show me the navigation buttons\"\n");
    printf("- \"What can I do on this page?\"\n");
    printf("\n");
}

void show_status(ChatSession *session) {
    if (!session) return;

    printf("=== Current Session Status ===\n");
    printf("User ID: %s\n", session->user_id);
    printf("Role: %s\n", session->role);
    printf("Language: %s\n", session->language);
    printf("Current Route: %s\n", session->context);
    printf("Message Count: %d\n", session->message_count);
    printf("Session ID: %s\n", session->id);
    printf("\n");
}

void clear_screen(void) {
    printf("\033[2J\033[H");
}

static bool is_valid_role(const char *role) {
    if (!role) return false;
    return strcmp(role, "tenant") == 0 || strcmp(role, "caretaker") == 0 || strcmp(role, "manager") == 0 || strcmp(role, "admin") == 0;
}

static bool is_valid_language(const char *language) {
    if (!language) return false;
    return strcmp(language, "en") == 0 || strcmp(language, "zu") == 0;
}

static const char *default_route_for_role(const char *role) {
    if (!role) return "/";
    if (strcmp(role, "caretaker") == 0) return "/caretaker";
    if (strcmp(role, "manager") == 0) return "/manager";
    if (strcmp(role, "admin") == 0) return "/admin";
    return "/tenant";
}

bool process_command(const char *input, ChatSession **session) {
    if (!input || input[0] != '/') return false;

    char *command = strdup(input);
    if (!command) return false;

    char *token = strtok(command, " ");
    if (!token) {
        free(command);
        return false;
    }

    if (strcmp(token, "/quit") == 0) {
        free(command);
        return true;
    } else if (strcmp(token, "/help") == 0) {
        show_help();
    } else if (strcmp(token, "/clear") == 0) {
        clear_screen();
    } else if (strcmp(token, "/role") == 0) {
        char *role = strtok(NULL, " ");
        if (role && (strcmp(role, "tenant") == 0 ||
                     strcmp(role, "caretaker") == 0 ||
                     strcmp(role, "manager") == 0 ||
                     strcmp(role, "admin") == 0)) {
            free((*session)->role);
            (*session)->role = strdup(role);
            printf("Role changed to: %s\n", role);
        } else {
            printf("Invalid role. Use: tenant, caretaker, manager, or admin\n");
        }
    } else if (strcmp(token, "/lang") == 0) {
        char *lang = strtok(NULL, " ");
        if (lang && (strcmp(lang, "en") == 0 || strcmp(lang, "zu") == 0)) {
            free((*session)->language);
            (*session)->language = strdup(lang);
            printf("Language changed to: %s\n", lang);
        } else {
            printf("Invalid language. Use: en or zu\n");
        }
    } else if (strcmp(token, "/route") == 0) {
        char *route = strtok(NULL, " ");
        if (route) {
            free((*session)->context);
            (*session)->context = strdup(route);
            printf("Current route set to: %s\n", route);

            NavigationContext *nav_ctx = get_navigation_context(route, (*session)->role);
            if (nav_ctx) {
                printf("Available buttons: ");
                for (int i = 0; i < nav_ctx->button_count; i++) {
                    printf("[%s]", nav_ctx->available_buttons[i]);
                    if (i < nav_ctx->button_count - 1) printf(" ");
                }
                printf("\n");
                free_navigation_context(nav_ctx);
            }
        } else {
            printf("Usage: /route <path> (e.g., /tenant/payments)\n");
        }
    } else if (strcmp(token, "/status") == 0) {
        show_status(*session);
    } else if (strcmp(token, "/stats") == 0) {
        cache_stats();
    } else {
        printf("Unknown command: %s\n", token);
        printf("Type /help for available commands\n");
    }

    free(command);
    return false;
}

static void emit_json_string(const char *value) {
    putchar('"');
    if (value) {
        for (const unsigned char *ptr = (const unsigned char *)value; *ptr; ptr++) {
            if (*ptr == '"' || *ptr == '\\') {
                printf("\\%c", *ptr);
            } else if (*ptr <= 0x1F) {
                printf("\\u%04x", *ptr);
            } else {
                putchar(*ptr);
            }
        }
    }
    putchar('"');
}

static void print_json_payload(const char *response_text, float confidence, long response_time_ms, const char *language, const char *role) {
    printf("{");
    printf("\"response\":");
    emit_json_string(response_text ? response_text : "");
    printf(",\"confidence\":%.2f", confidence);
    printf(",\"responseTime\":%ld", response_time_ms);
    printf(",\"language\":");
    emit_json_string(language ? language : "");
    printf(",\"role\":");
    emit_json_string(role ? role : "");
    printf("}\n");
}

static long calculate_response_time_ms(clock_t start_clock, clock_t end_clock) {
    if (start_clock == (clock_t)-1 || end_clock == (clock_t)-1) {
        return 0;
    }
    double elapsed_seconds = (double)(end_clock - start_clock) / (double)CLOCKS_PER_SEC;
    if (elapsed_seconds < 0) {
        return 0;
    }
    double elapsed_ms = elapsed_seconds * 1000.0;
    if (elapsed_ms < 0) {
        return 0;
    }
    return (long)(elapsed_ms + 0.5);
}

int main(int argc, char **argv) {
    const char *role = "tenant";
    const char *language = "en";
    const char *route = NULL;
    const char *single_query = NULL;
    bool json_output = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(arg, "--single-query") == 0 || strcmp(arg, "-q") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing message for %s\n", arg);
                return 1;
            }
            single_query = argv[++i];
        } else if (strcmp(arg, "--role") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --role\n");
                return 1;
            }
            role = argv[++i];
            if (!is_valid_role(role)) {
                fprintf(stderr, "Error: Invalid role '%s'\n", role);
                return 1;
            }
        } else if (strcmp(arg, "--lang") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --lang\n");
                return 1;
            }
            language = argv[++i];
            if (!is_valid_language(language)) {
                fprintf(stderr, "Error: Invalid language '%s'\n", language);
                return 1;
            }
        } else if (strcmp(arg, "--route") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --route\n");
                return 1;
            }
            route = argv[++i];
        } else if (strcmp(arg, "--json-output") == 0 || strcmp(arg, "-j") == 0) {
            json_output = true;
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", arg);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!is_valid_role(role)) {
        fprintf(stderr, "Error: Invalid role '%s'\n", role);
        return 1;
    }
    if (!is_valid_language(language)) {
        fprintf(stderr, "Error: Invalid language '%s'\n", language);
        return 1;
    }

    if (!single_query) {
        printf("=== Bricllm - Briconomy Navigation Assistant ===\n");
        printf("High-performance console chatbot for Briconomy app navigation\n");
        printf("Type 'help' for commands or ask me about the app!\n\n");
    }

    init_chat_engine();
    init_route_system();

    ChatSession *current_session = create_session("console_user", role, language);
    if (!current_session) {
        fprintf(stderr, "Error: Failed to create session\n");
        return 1;
    }

    const char *initial_route = route ? route : default_route_for_role(role);
    free(current_session->context);
    current_session->context = strdup(initial_route);
    if (!current_session->context) {
        fprintf(stderr, "Error: Failed to set session route\n");
        free_session(current_session);
        return 1;
    }

    if (!single_query) {
        printf("Session created. Default role: %s, language: %s\n", role, language);
        printf("Use /role to change role, /route to set current page context\n\n");
    }

    if (single_query) {
        clock_t start_clock = clock();
        ChatResponse *response = process_message(current_session, single_query);
        clock_t end_clock = clock();
        long response_time_ms = calculate_response_time_ms(start_clock, end_clock);

        if (!response) {
            if (json_output) {
                print_json_payload("Unable to process request", 0.0f, response_time_ms, language, role);
            } else {
                printf("Bricllm: Unable to process request\n");
            }
            free_session(current_session);
            return 1;
        }

        if (json_output) {
            print_json_payload(response->response ? response->response : "", response->confidence, response_time_ms, language, role);
        } else {
            printf("Bricllm: %s\n", response->response ? response->response : "");
            if (response->suggested_actions && response->action_count > 0) {
                printf("\nSuggested actions:\n");
                for (int i = 0; i < response->action_count; i++) {
                    SuggestedAction *action = response->suggested_actions[i];
                    printf("  • %s\n", action && action->label ? action->label : "");
                }
            }
            if (response->confidence < 0.7f && response->confidence > 0.0f) {
                printf("\n(Confidence: %.0f%% - I'm not entirely sure about this answer)\n", response->confidence * 100);
            }
            if (response->escalation_needed) {
                printf("\nWARNING: I'm having trouble understanding. Would you like me to connect you with human support?\n");
            }
        }

        int exit_code = response->escalation_needed ? 2 : 0;
        free_response(response);
        free_session(current_session);
        return exit_code;
    }

    char input[1024];
    bool should_quit = false;

    while (!should_quit) {
        printf("You: ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) continue;

        if (input[0] == '/') {
            should_quit = process_command(input, &current_session);
            if (!should_quit) {
                printf("\n");
            }
            continue;
        }

        clock_t start_clock = clock();
        ChatResponse *response = process_message(current_session, input);
        clock_t end_clock = clock();
        long response_time_ms = calculate_response_time_ms(start_clock, end_clock);

        if (response) {
            if (json_output) {
                print_json_payload(response->response ? response->response : "", response->confidence, response_time_ms, current_session->language, current_session->role);
            } else {
                printf("Bricllm: %s\n", response->response ? response->response : "");

                if (response->suggested_actions && response->action_count > 0) {
                    printf("\nSuggested actions:\n");
                    for (int i = 0; i < response->action_count; i++) {
                        SuggestedAction *action = response->suggested_actions[i];
                        printf("  • %s\n", action && action->label ? action->label : "");
                    }
                }

                if (response->confidence < 0.7f && response->confidence > 0.0f) {
                    printf("\n(Confidence: %.0f%% - I'm not entirely sure about this answer)\n",
                           response->confidence * 100);
                }

                if (response->escalation_needed) {
                    printf("\nWARNING: I'm having trouble understanding. Would you like me to connect you with human support?\n");
                }
            }

            free_response(response);
        } else {
            if (json_output) {
                print_json_payload("I'm sorry, I didn't understand that", 0.0f, response_time_ms, current_session->language, current_session->role);
            } else {
                printf("Bricllm: I'm sorry, I didn't understand that. Could you please rephrase your question?\n");
                printf("You can ask about rent payments, maintenance requests, navigation help, or type /help for commands.\n");
            }
        }

        if (!json_output) {
            printf("\n");
        }
    }

    printf("\nGoodbye! Thank you for using Bricllm.\n");
    free_session(current_session);

    return 0;
}