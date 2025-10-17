#include "../../include/route_types.h"
#include "../../include/bricllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RouteGuide *tenant_route_guides = NULL;
static int tenant_route_count = 0;

void load_tenant_routes(void) {
    static char *tenant_nav_buttons[] = {"Home", "Payments", "Requests", "Profile"};
    static char *dashboard_actions[] = {"View Dashboard", "Check Notifications", "Recent Activity"};
    static char *payment_actions[] = {"Pay Rent", "View History", "Payment Methods", "Due Dates"};
    static char *request_actions[] = {"New Request", "View Status", "Request History"};
    static char *profile_actions[] = {"Edit Profile", "Change Password", "Contact Info", "Documents"};

    static char route_tenant[] = "/tenant";
    static char route_payments[] = "/tenant/payments";
    static char route_requests[] = "/tenant/requests";
    static char route_profile[] = "/tenant/profile";
    static char role_tenant[] = "tenant";

    tenant_route_count = 4;
    tenant_route_guides = malloc(tenant_route_count * sizeof(RouteGuide));
    if (!tenant_route_guides) {
        log_message("ERROR", "Failed to allocate memory for tenant routes");
        return;
    }

    tenant_route_guides[0].route = route_tenant;
    tenant_route_guides[0].user_role = role_tenant;
    tenant_route_guides[0].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[0].page_actions = dashboard_actions;
    tenant_route_guides[0].patterns = NULL;
    tenant_route_guides[0].pattern_count = 2;

    tenant_route_guides[1].route = route_payments;
    tenant_route_guides[1].user_role = role_tenant;
    tenant_route_guides[1].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[1].page_actions = payment_actions;
    tenant_route_guides[1].patterns = NULL;
    tenant_route_guides[1].pattern_count = 3;

    tenant_route_guides[2].route = route_requests;
    tenant_route_guides[2].user_role = role_tenant;
    tenant_route_guides[2].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[2].page_actions = request_actions;
    tenant_route_guides[2].patterns = NULL;
    tenant_route_guides[2].pattern_count = 2;

    tenant_route_guides[3].route = route_profile;
    tenant_route_guides[3].user_role = role_tenant;
    tenant_route_guides[3].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[3].page_actions = profile_actions;
    tenant_route_guides[3].patterns = NULL;
    tenant_route_guides[3].pattern_count = 0;

    log_message("INFO", "Loaded %d tenant routes", tenant_route_count);
}

RouteGuide *find_tenant_route(const char *route) {
    for (int i = 0; i < tenant_route_count; i++) {
        if (strcmp(tenant_route_guides[i].route, route) == 0) {
            return &tenant_route_guides[i];
        }
    }
    return NULL;
}