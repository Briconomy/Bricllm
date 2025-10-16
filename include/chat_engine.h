#ifndef CHAT_ENGINE_H
#define CHAT_ENGINE_H

#include "bricllm.h"

void init_chat_engine(void);

ChatSession *create_session(const char *user_id, const char *role, const char *language);
ChatSession *find_session(const char *session_id);
void free_session(ChatSession *session);
void cleanup_expired_sessions(void);

ChatResponse *process_message(ChatSession *session, const char *message);
void free_response(ChatResponse *response);

#endif // CHAT_ENGINE_H