#include "../../include/bricllm.h"
#include "../../include/chat_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static ChatSession **sessions = NULL;
static int session_count = 0;
static int max_sessions = 1000;

static ResponsePattern *patterns = NULL;
static int pattern_count = 0;

static void load_response_patterns(void);
static char *generate_session_id(void);
static ChatResponse *create_response_from_pattern(ResponsePattern *pattern, const char *message);

void init_chat_engine(void) {
    srand(time(NULL));

    sessions = malloc(max_sessions * sizeof(ChatSession *));
    if (!sessions) {
        log_message("ERROR", "Failed to allocate memory for sessions");
        exit(1);
    }

    load_response_patterns();
    log_message("INFO", "Chat engine initialized with %d patterns", pattern_count);
}

ChatResponse *process_message(ChatSession *session, const char *message) {
    if (!session || !message) {
        return NULL;
    }

    session->last_activity = time(NULL);
    session->message_count++;

    log_message("INFO", "Processing message from user %s: %.50s",
                session->user_id, message);

    ResponsePattern *pattern = find_matching_pattern(message, session->role, session->language);

    ChatResponse *response;
    if (pattern) {
        response = create_response_from_pattern(pattern, message);
        if (!response) {
            free(pattern->response);
            free(pattern->category);
            free(pattern);
            return NULL;
        }

        log_message("INFO", "Found matching pattern: %s (confidence: %.2f)",
                    pattern->category, response->confidence);

        free(pattern->response);
        free(pattern->category);
        free(pattern);
    } else {
        response = malloc(sizeof(ChatResponse));
        if (!response) return NULL;

        response->message_id = generate_uuid();
        response->confidence = 0.0f;
        response->escalation_needed = false;
        response->suggested_actions = NULL;
        response->action_count = 0;

        static const char *tenant_zu_responses[] = {
            "Angiqondi lowo mbuzo. Ungazama ukuwushisa kabusha?",
            "Ngicela ungichazele kabanzi. Ngingakusiza kanjani?",
            "Angikwazi ukuphendula lokho. Ungathanda ukubuza ngento ethile ye-app?"
        };
        
        static const char *tenant_en_responses[] = {
            "I'm not sure about that. Could you rephrase your question or ask about something specific with the app?",
            "I didn't quite understand that. What would you like help with in the Briconomy app?",
            "Hmm, I'm not able to help with that. Is there something about payments, requests, or navigation I can assist you with?"
        };
        
        static const char *caretaker_responses[] = {
            "I can help you with tasks, schedule, and work history. What would you like to know?",
            "I'm here to assist with your caretaker duties. What do you need help with?",
            "Let me help you navigate your caretaker features. What are you looking for?"
        };
        
        static const char *manager_responses[] = {
            "I can assist with property management, leases, and reports. How can I help?",
            "I'm here to help manage your properties. What do you need?",
            "Let me help you with your management tasks. What would you like to do?"
        };
        
        static const char *admin_responses[] = {
            "I can help with user management, security settings, and system administration. What do you need?",
            "I'm here to assist with admin functions. What would you like to configure?",
            "Let me help you with system administration. What are you looking for?"
        };
        
        static const char *default_responses[] = {
            "I'm here to help you navigate the Briconomy app. What would you like assistance with?",
            "Let me help you with the app. What are you trying to do?",
            "I can guide you through the Briconomy features. What do you need?"
        };

        int variation = rand() % 3;
        const char *selected_response;
        
        if (strcmp(session->role, "tenant") == 0) {
            if (strcmp(session->language, "zu") == 0) {
                selected_response = tenant_zu_responses[variation];
            } else {
                selected_response = tenant_en_responses[variation];
            }
        } else if (strcmp(session->role, "caretaker") == 0) {
            selected_response = caretaker_responses[variation];
        } else if (strcmp(session->role, "manager") == 0) {
            selected_response = manager_responses[variation];
        } else if (strcmp(session->role, "admin") == 0) {
            selected_response = admin_responses[variation];
        } else {
            selected_response = default_responses[variation];
        }
        
        response->response = strdup(selected_response);
        response->response_type = "text";

        log_message("WARN", "No matching pattern found for user %s", session->user_id);
    }

    return response;
}

ChatSession *create_session(const char *user_id, const char *role, const char *language) {
    if (!user_id || !role || !language) {
        return NULL;
    }

    if (session_count >= max_sessions) {
        cleanup_expired_sessions();
        if (session_count >= max_sessions) {
            log_message("ERROR", "Maximum session limit reached");
            return NULL;
        }
    }

    ChatSession *session = malloc(sizeof(ChatSession));
    if (!session) return NULL;

    session->id = generate_session_id();
    if (!session->id) {
        free(session);
        return NULL;
    }

    session->user_id = strdup(user_id);
    if (!session->user_id) {
        free(session->id);
        free(session);
        return NULL;
    }

    session->role = strdup(role);
    if (!session->role) {
        free(session->id);
        free(session->user_id);
        free(session);
        return NULL;
    }

    session->language = strdup(language);
    if (!session->language) {
        free(session->id);
        free(session->user_id);
        free(session->role);
        free(session);
        return NULL;
    }

    session->created_at = time(NULL);
    session->last_activity = time(NULL);
    session->message_count = 0;
    session->context = strdup("/");
    if (!session->context) {
        free(session->id);
        free(session->user_id);
        free(session->role);
        free(session->language);
        free(session);
        return NULL;
    }

    sessions[session_count++] = session;

    log_message("INFO", "Created new session %s for user %s (role: %s)",
                session->id, user_id, role);

    return session;
}

void free_session(ChatSession *session) {
    if (!session) return;

    free(session->id);
    free(session->user_id);
    free(session->role);
    free(session->language);
    free(session->context);
    free(session);
}

ChatSession *find_session(const char *session_id) {
    if (!session_id) return NULL;

    for (int i = 0; i < session_count; i++) {
        if (sessions[i] && strcmp(sessions[i]->id, session_id) == 0) {
            return sessions[i];
        }
    }
    return NULL;
}

void cleanup_expired_sessions(void) {
    time_t now = time(NULL);
    int timeout = 3600;

    for (int i = 0; i < session_count; i++) {
        if (sessions[i] && (now - sessions[i]->last_activity) > timeout) {
            log_message("INFO", "Cleaning up expired session %s", sessions[i]->id);
            free_session(sessions[i]);
            sessions[i] = NULL;
        }
    }

    int write_index = 0;
    for (int i = 0; i < session_count; i++) {
        if (sessions[i]) {
            sessions[write_index++] = sessions[i];
        }
    }
    session_count = write_index;
}

static void load_response_patterns(void) {
    static char *nav_keywords[] = {"where", "find", "navigate", "how to"};
    static char *payment_keywords[] = {"rent", "pay", "payment", "due"};

    pattern_count = 2;
    patterns = malloc(pattern_count * sizeof(ResponsePattern));
    if (!patterns) {
        log_message("ERROR", "Failed to allocate memory for patterns");
        return;
    }

    patterns[0].keywords = nav_keywords;
    patterns[0].keyword_count = 4;
    patterns[0].response = "I can help you navigate. Use the bottom navigation buttons to access different sections of the app.";
    patterns[0].category = "navigation";
    patterns[0].user_role = "all";
    patterns[0].language = "en";
    patterns[0].confidence_threshold = 0.7f;

    patterns[1].keywords = payment_keywords;
    patterns[1].keyword_count = 4;
    patterns[1].response = "To pay your rent: 1) Go to Payments section 2) Select your property 3) Choose payment method 4) Confirm payment";
    patterns[1].category = "payment";
    patterns[1].user_role = "tenant";
    patterns[1].language = "en";
    patterns[1].confidence_threshold = 0.8f;

    log_message("INFO", "Loaded %d response patterns", pattern_count);
}

static char *generate_session_id(void) {
    char *session_id = malloc(33);
    if (!session_id) return NULL;

    snprintf(session_id, 33, "%08x%04x%04x%04x%012llx",
             (unsigned int)rand(), (unsigned int)rand() % 0xFFFF, (unsigned int)rand() % 0xFFFF,
             (unsigned int)rand() % 0xFFFF, (long long)((long long)rand() << 32) | rand());

    return session_id;
}

static ChatResponse *create_response_from_pattern(ResponsePattern *pattern, const char *message) {
    static char action_type_nav[] = "navigation";
    static char action_label_dash[] = "View Dashboard";
    static char action_target_dash[] = "/dashboard";

    ChatResponse *response = malloc(sizeof(ChatResponse));
    if (!response) return NULL;

    response->message_id = generate_uuid();
    response->response = strdup(pattern->response);
    response->response_type = strdup(pattern->category);

    if (pattern->keywords && pattern->keyword_count > 0) {
        response->confidence = calculate_similarity(message, pattern->keywords[0]);
    } else {
        response->confidence = 0.8f;
    }

    response->escalation_needed = false;
    response->suggested_actions = NULL;
    response->action_count = 0;

    if (strcmp(pattern->category, "navigation") == 0) {
        response->suggested_actions = malloc(2 * sizeof(SuggestedAction *));
        response->action_count = 1;

        SuggestedAction *action = malloc(sizeof(SuggestedAction));
        action->type = action_type_nav;
        action->label = action_label_dash;
        action->target = action_target_dash;

        response->suggested_actions[0] = action;
    }

    return response;
}

void free_response(ChatResponse *response) {
    if (!response) return;

    free(response->message_id);
    free(response->response);
    free(response->response_type);

    if (response->suggested_actions) {
        for (int i = 0; i < response->action_count; i++) {
            if (response->suggested_actions[i]) {
                // #COMPLETION_DRIVE: Assuming action fields point to static strings or need freeing
                // #SUGGEST_VERIFY: Track allocation source to determine if free is needed
                free(response->suggested_actions[i]);
            }
        }
        free(response->suggested_actions);
    }

    free(response);
}