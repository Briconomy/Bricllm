#include "include/bricllm.h"
#include "include/chat_engine.h"
#include "include/route_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void show_help(void) {
    printf("=== Bricllm Commands ===\n");
    printf("/help                 - Show this help message\n");
    printf("/clear                - Clear screen\n");
    printf("/role <role>          - Set user role (tenant, caretaker, manager, admin)\n");
    printf("/lang <lang>          - Set language (en, zu)\n");
    printf("/route <path>         - Set current route context\n");
    printf("/status               - Show current session status\n");
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
    } else {
        printf("Unknown command: %s\n", token);
        printf("Type /help for available commands\n");
    }

    free(command);
    return false;
}

int main() {
    printf("=== Bricllm - Briconomy Navigation Assistant ===\n");
    printf("High-performance console chatbot for Briconomy app navigation\n");
    printf("Type 'help' for commands or ask me about the app!\n\n");

    init_chat_engine();
    init_route_system();

    ChatSession *current_session = create_session("console_user", "tenant", "en");
    if (!current_session) {
        printf("Error: Failed to create session\n");
        return 1;
    }

    free(current_session->context);
    current_session->context = strdup("/tenant");

    printf("Session created. Default role: tenant, language: en\n");
    printf("Use /role to change role, /route to set current page context\n\n");

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

        ChatResponse *response = process_message(current_session, input);
        if (response) {
            printf("Bricllm: %s\n", response->response);

            if (response->suggested_actions && response->action_count > 0) {
                printf("\nSuggested actions:\n");
                for (int i = 0; i < response->action_count; i++) {
                    SuggestedAction *action = response->suggested_actions[i];
                    printf("  • %s\n", action->label);
                }
            }

            if (response->confidence < 0.7f && response->confidence > 0.0f) {
                printf("\n(Confidence: %.0f%% - I'm not entirely sure about this answer)\n",
                       response->confidence * 100);
            }

            if (response->escalation_needed) {
                printf("\nWARNING: I'm having trouble understanding. Would you like me to connect you with human support?\n");
            }

            free_response(response);
        } else {
            printf("Bricllm: I'm sorry, I didn't understand that. Could you please rephrase your question?\n");
            printf("You can ask about rent payments, maintenance requests, navigation help, or type /help for commands.\n");
        }

        printf("\n");
    }

    printf("\nGoodbye! Thank you for using Bricllm.\n");
    free_session(current_session);

    return 0;
}