#include "../include/conversation_context.h"
#include "../include/bricllm.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

ConversationContext *create_conversation_context(void) {
    ConversationContext *ctx = malloc(sizeof(ConversationContext));
    if (!ctx) return NULL;
    
    ctx->last_topic = NULL;
    ctx->last_entity = NULL;
    ctx->last_action = NULL;
    ctx->last_options = NULL;
    ctx->option_count = 0;
    ctx->history_count = 0;
    ctx->last_intent = NULL;
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        ctx->message_history[i] = NULL;
    }
    
    log_message("INFO", "Created conversation context");
    return ctx;
}

void free_conversation_context(ConversationContext *ctx) {
    if (!ctx) return;
    
    free(ctx->last_topic);
    free(ctx->last_entity);
    free(ctx->last_action);
    free(ctx->last_intent);
    
    if (ctx->last_options) {
        for (int i = 0; i < ctx->option_count; i++) {
            free(ctx->last_options[i]);
        }
        free(ctx->last_options);
    }
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        free(ctx->message_history[i]);
    }
    
    free(ctx);
}

void update_context(ConversationContext *ctx, const char *topic, 
                   const char *entity, const char *action) {
    if (!ctx) return;
    
    if (topic) {
        free(ctx->last_topic);
        ctx->last_topic = strdup(topic);
    }
    
    if (entity) {
        free(ctx->last_entity);
        ctx->last_entity = strdup(entity);
    }
    
    if (action) {
        free(ctx->last_action);
        ctx->last_action = strdup(action);
    }
    
    log_message("INFO", "Context updated - topic: %s, entity: %s, action: %s",
               topic ? topic : "none",
               entity ? entity : "none",
               action ? action : "none");
}

void add_to_history(ConversationContext *ctx, const char *message) {
    if (!ctx || !message) return;
    
    // If history is full, shift everything down
    if (ctx->history_count >= MAX_HISTORY) {
        free(ctx->message_history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            ctx->message_history[i] = ctx->message_history[i + 1];
        }
        ctx->history_count = MAX_HISTORY - 1;
    }
    
    // Add new message at the end
    ctx->message_history[ctx->history_count++] = strdup(message);
}

void set_context_options(ConversationContext *ctx, char **options, int count) {
    if (!ctx) return;
    
    if (ctx->last_options) {
        for (int i = 0; i < ctx->option_count; i++) {
            free(ctx->last_options[i]);
        }
        free(ctx->last_options);
    }
    
    ctx->last_options = malloc(count * sizeof(char*));
    ctx->option_count = count;
    
    for (int i = 0; i < count; i++) {
        ctx->last_options[i] = strdup(options[i]);
    }
}

static int contains_pronoun(const char *message) {
    static const char *pronouns[] = {
        "it", "that", "there", "this", "which",
        "same", "also", "too", "another"
    };
    
    char lower_msg[256];
    strncpy(lower_msg, message, sizeof(lower_msg) - 1);
    lower_msg[sizeof(lower_msg) - 1] = '\0';
    
    for (int i = 0; lower_msg[i]; i++) {
        lower_msg[i] = tolower(lower_msg[i]);
    }
    
    for (int i = 0; i < 9; i++) {
        if (strstr(lower_msg, pronouns[i])) {
            return 1;
        }
    }
    
    return 0;
}

char *resolve_pronoun(ConversationContext *ctx, const char *message) {
    if (!ctx || !message || !contains_pronoun(message)) {
        return NULL;
    }
    
    static char resolved[512];
    strncpy(resolved, message, sizeof(resolved) - 1);
    resolved[sizeof(resolved) - 1] = '\0';
    
    char lower_msg[256];
    strncpy(lower_msg, message, sizeof(lower_msg) - 1);
    lower_msg[sizeof(lower_msg) - 1] = '\0';
    
    for (int i = 0; lower_msg[i]; i++) {
        lower_msg[i] = tolower(lower_msg[i]);
    }
    
    if (strstr(lower_msg, "how do i do it") || 
        strstr(lower_msg, "how do i do that") ||
        strstr(lower_msg, "do it") ||
        strstr(lower_msg, "do that")) {
        
        if (ctx->last_action) {
            snprintf(resolved, sizeof(resolved), "How do I %s?", ctx->last_action);
            log_message("INFO", "Resolved pronoun 'it/that' -> '%s'", ctx->last_action);
            return strdup(resolved);
        }
    }
    
    if (strstr(lower_msg, "where is it") || 
        strstr(lower_msg, "where is that") ||
        strstr(lower_msg, "get there")) {
        
        if (ctx->last_entity) {
            snprintf(resolved, sizeof(resolved), "Where is %s?", ctx->last_entity);
            log_message("INFO", "Resolved pronoun 'it/that/there' -> '%s'", ctx->last_entity);
            return strdup(resolved);
        }
    }
    
    if (strstr(lower_msg, "which") && ctx->option_count > 0) {
        snprintf(resolved, sizeof(resolved), "Tell me about %s options", 
                ctx->last_topic ? ctx->last_topic : "the");
        log_message("INFO", "Resolved 'which' -> asking about %s options", 
                   ctx->last_topic ? ctx->last_topic : "the");
        return strdup(resolved);
    }
    
    if ((strstr(lower_msg, "also") || strstr(lower_msg, "too")) && ctx->last_topic) {
        log_message("INFO", "Detected 'also/too' - previous context: %s", ctx->last_topic);
    }
    
    if (strstr(lower_msg, "same") && ctx->last_topic) {
        snprintf(resolved, sizeof(resolved), "%s", ctx->last_topic);
        log_message("INFO", "Resolved 'same' -> '%s'", ctx->last_topic);
        return strdup(resolved);
    }
    
    if (strstr(lower_msg, "another") && ctx->last_entity) {
        snprintf(resolved, sizeof(resolved), "another %s", ctx->last_entity);
        log_message("INFO", "Resolved 'another' -> 'another %s'", ctx->last_entity);
        return strdup(resolved);
    }
    
    if ((strstr(lower_msg, "what about") || strstr(lower_msg, "how about")) && ctx->last_topic) {
        log_message("INFO", "Detected comparison question about: %s", ctx->last_topic);
    }
    
    return NULL;
}
