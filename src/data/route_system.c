#include "../../include/route_types.h"
#include "../../include/bricllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void load_tenant_routes(void);
extern RouteGuide *find_tenant_route(const char *route);
extern RouteGuide *tenant_route_guides;
extern int tenant_route_count;

void init_route_system(void) {
    log_message("INFO", "Initializing route system");

    load_tenant_routes();

    log_message("INFO", "Route system initialized");
}

RouteGuide *find_route_guide(const char *route, const char *user_role) {
    if (!route || !user_role) {
        return NULL;
    }

    if (strcmp(user_role, "tenant") == 0) {
        return find_tenant_route(route);
    }

    return find_tenant_route(route);
}

NavigationContext *get_navigation_context(const char *route, const char *user_role) {
    if (!route || !user_role) {
        return NULL;
    }

    NavigationContext *ctx = malloc(sizeof(NavigationContext));
    if (!ctx) return NULL;

    ctx->current_route = strdup(route);
    if (!ctx->current_route) {
        free(ctx);
        return NULL;
    }

    ctx->user_role = strdup(user_role);
    if (!ctx->user_role) {
        free(ctx->current_route);
        free(ctx);
        return NULL;
    }

    ctx->available_buttons = NULL;
    ctx->button_count = 0;
    ctx->suggested_actions = NULL;
    ctx->action_count = 0;

    RouteGuide *guide = find_route_guide(route, user_role);
    if (guide) {
        ctx->available_buttons = guide->navigation_buttons;
        ctx->button_count = 4;
        ctx->suggested_actions = guide->page_actions;
        ctx->action_count = 3;
    } else {
        if (strcmp(user_role, "tenant") == 0) {
            static char *default_buttons[] = {"Home", "Payments", "Requests", "Profile"};
            ctx->available_buttons = default_buttons;
            ctx->button_count = 4;

            static char *default_actions[] = {"View Dashboard", "Check Notifications", "Help"};
            ctx->suggested_actions = default_actions;
            ctx->action_count = 3;
        } else if (strcmp(user_role, "caretaker") == 0) {
            static char *default_buttons[] = {"Tasks", "Schedule", "History", "Profile"};
            ctx->available_buttons = default_buttons;
            ctx->button_count = 4;

            static char *default_actions[] = {"View Tasks", "Check Schedule", "Help"};
            ctx->suggested_actions = default_actions;
            ctx->action_count = 3;
        } else if (strcmp(user_role, "manager") == 0) {
            static char *default_buttons[] = {"Dashboard", "Properties", "Leases", "Payments"};
            ctx->available_buttons = default_buttons;
            ctx->button_count = 4;

            static char *default_actions[] = {"Manage Properties", "View Reports", "Help"};
            ctx->suggested_actions = default_actions;
            ctx->action_count = 3;
        } else if (strcmp(user_role, "admin") == 0) {
            static char *default_buttons[] = {"Dashboard", "Users", "Security", "Reports"};
            ctx->available_buttons = default_buttons;
            ctx->button_count = 3;

            static char *default_actions[] = {"User Management", "System Settings", "Help"};
            ctx->suggested_actions = default_actions;
            ctx->action_count = 3;
        }
    }

    log_message("INFO", "Created navigation context for %s on route %s",
                user_role, route);

    return ctx;
}

void free_navigation_context(NavigationContext *ctx) {
    if (!ctx) return;

    free(ctx->current_route);
    free(ctx->user_role);
    free(ctx);
}

void load_caretaker_routes(void) {
    log_message("INFO", "Caretaker routes not yet implemented");
}

void load_manager_routes(void) {
    log_message("INFO", "Manager routes not yet implemented");
}

void load_admin_routes(void) {
    log_message("INFO", "Admin routes not yet implemented");
}
