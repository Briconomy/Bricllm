#ifndef BRICLLM_H
#define BRICLLM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Forward declaration
typedef struct ConversationContext ConversationContext;

typedef struct {
    char *id;
    char *user_id;
    char *role;
    char *language;
    time_t created_at;
    time_t last_activity;
    int message_count;
    char *context;
    ConversationContext *conv_context;  // Conversation memory
} ChatSession;

typedef struct {
    char *message_id;
    char *session_id;
    char *text;
    char sender_type;
    time_t timestamp;
    char *response_type;
} ChatMessage;

typedef struct {
    char **keywords;
    int keyword_count;
    char *response;
    char *category;
    char *user_role;
    char *language;
    float confidence_threshold;
} ResponsePattern;

typedef enum {
    ACTION_STATIC,
    ACTION_ALLOCATED
} ActionAllocationType;

typedef struct {
    char *type;
    char *label;
    char *target;
    ActionAllocationType allocation_type;
} SuggestedAction;

typedef struct {
    char *message_id;
    char *response;
    char *response_type;
    float confidence;
    bool escalation_needed;
    SuggestedAction **suggested_actions;
    int action_count;
} ChatResponse;

ChatSession *create_session(const char *user_id, const char *role, const char *language);
void free_session(ChatSession *session);
ChatMessage *create_message(const char *session_id, const char *text, char sender_type);
void free_message(ChatMessage *message);
ChatResponse *process_message(ChatSession *session, const char *message);
void free_response(ChatResponse *response);

ResponsePattern *find_matching_pattern(const char *message, const char *role, const char *language);
float calculate_similarity(const char *str1, const char *str2);

ChatSession *find_session(const char *session_id);
void cleanup_expired_sessions(void);

char *generate_uuid(void);
char *get_timestamp(void);
void log_message(const char *level, const char *format, ...);

#endif // BRICLLM_H