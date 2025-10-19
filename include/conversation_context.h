#ifndef CONVERSATION_CONTEXT_H
#define CONVERSATION_CONTEXT_H

#define MAX_HISTORY 5
#define MAX_CONTEXT_STRING 128

// Use the forward declaration from bricllm.h
struct ConversationContext {
    char *last_topic;          // "payment", "maintenance", "navigation"
    char *last_entity;         // "Payments section", "rent", "maintenance request"
    char *last_action;         // "pay", "report", "navigate", "find"
    char **last_options;       // ["Card", "Bank", "Cash"] for "which one?"
    int option_count;
    char *message_history[MAX_HISTORY];  // Last 5 messages
    int history_count;
    char *last_intent;         // "pay_rent", "report_maintenance", "navigate"
};

typedef struct ConversationContext ConversationContext;

ConversationContext *create_conversation_context(void);
void free_conversation_context(ConversationContext *ctx);
void update_context(ConversationContext *ctx, const char *topic, 
                   const char *entity, const char *action);
void add_to_history(ConversationContext *ctx, const char *message);
void set_context_options(ConversationContext *ctx, char **options, int count);
char *resolve_pronoun(ConversationContext *ctx, const char *message);

#endif // CONVERSATION_CONTEXT_H
