#ifndef ROUTE_TYPES_H
#define ROUTE_TYPES_H

#include "bricllm.h"

typedef struct {
    char *route;
    char *user_role;
    ResponsePattern **patterns;
    int pattern_count;
    char **navigation_buttons;
    char **page_actions;
} RouteGuide;

typedef struct {
    char *current_route;
    char *user_role;
    char **available_buttons;
    int button_count;
    char **suggested_actions;
    int action_count;
} NavigationContext;

void init_route_system(void);
RouteGuide *find_route_guide(const char *route, const char *user_role);
NavigationContext *get_navigation_context(const char *route, const char *user_role);
void free_navigation_context(NavigationContext *ctx);
void load_tenant_routes(void);
void load_caretaker_routes(void);
void load_manager_routes(void);
void load_admin_routes(void);

#endif // ROUTE_TYPES_H