#include "../../include/route_types.h"
#include "../../include/bricllm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static RouteGuide *tenant_route_guides = NULL;
static int tenant_route_count = 0;

static ResponsePattern *create_tenant_payment_patterns(void) {
    ResponsePattern *patterns = malloc(5 * sizeof(ResponsePattern));
    int count = 0;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(6 * sizeof(char*)),
        .keyword_count = 5,
        .response = strdup("Your rent is due on the 1st of each month. On the Payments page, you can: 1) View your current balance 2) Choose payment method 3) Confirm payment. Available buttons: [Home, Payments, Requests, Profile]"),
        .category = strdup("payment"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.8f
    };
    patterns[count].keywords[0] = strdup("rent");
    patterns[count].keywords[1] = strdup("pay");
    patterns[count].keywords[2] = strdup("payment");
    patterns[count].keywords[3] = strdup("due");
    patterns[count].keywords[4] = strdup("when");
    count++;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(4 * sizeof(char*)),
        .keyword_count = 3,
        .response = strdup("You can pay using: 1) Credit/Debit card 2) Bank transfer 3) Cash at property office. Go to Payments > Payment Methods to set up your preferred option."),
        .category = strdup("payment_method"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.8f
    };
    patterns[count].keywords[0] = strdup("how");
    patterns[count].keywords[1] = strdup("payment method");
    patterns[count].keywords[2] = strdup("pay rent");
    count++;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(4 * sizeof(char*)),
        .keyword_count = 3,
        .response = strdup("If you've missed a payment, please contact your property manager immediately. Late fees may apply after 7 days. You can still make payments through the Payments page."),
        .category = strdup("late_payment"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.8f
    };
    patterns[count].keywords[0] = strdup("late");
    patterns[count].keywords[1] = strdup("missed");
    patterns[count].keywords[2] = strdup("overdue");
    count++;

    return patterns;
}

static ResponsePattern *create_tenant_maintenance_patterns(void) {
    ResponsePattern *patterns = malloc(5 * sizeof(ResponsePattern));
    int count = 0;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(5 * sizeof(char*)),
        .keyword_count = 4,
        .response = strdup("To report maintenance issues: 1) Go to Requests page 2) Click 'New Request' 3) Describe the issue 4) Upload photos if possible 5) Submit. You'll receive updates on the status."),
        .category = strdup("maintenance"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.8f
    };
    patterns[count].keywords[0] = strdup("maintenance");
    patterns[count].keywords[1] = strdup("repair");
    patterns[count].keywords[2] = strdup("broken");
    patterns[count].keywords[3] = strdup("request");
    count++;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(5 * sizeof(char*)),
        .keyword_count = 4,
        .response = strdup("For emergencies like water leaks, gas leaks, or electrical issues: 1) Call emergency hotline immediately 2) Submit urgent maintenance request 3) Mark as emergency. Available buttons: [Home, Payments, Requests, Profile]"),
        .category = strdup("emergency"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.9f
    };
    patterns[count].keywords[0] = strdup("emergency");
    patterns[count].keywords[1] = strdup("urgent");
    patterns[count].keywords[2] = strdup("water leak");
    patterns[count].keywords[3] = strdup("electrical");
    count++;

    return patterns;
}

static ResponsePattern *create_tenant_navigation_patterns(void) {
    ResponsePattern *patterns = malloc(5 * sizeof(ResponsePattern));
    int count = 0;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(6 * sizeof(char*)),
        .keyword_count = 5,
        .response = strdup("As a tenant, your main navigation buttons are: [Home, Payments, Requests, Profile]. Home shows dashboard, Payments handles rent, Requests for maintenance, Profile for your account details."),
        .category = strdup("navigation"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.7f
    };
    patterns[count].keywords[0] = strdup("where");
    patterns[count].keywords[1] = strdup("find");
    patterns[count].keywords[2] = strdup("navigate");
    patterns[count].keywords[3] = strdup("how to");
    patterns[count].keywords[4] = strdup("buttons");
    count++;

    patterns[count] = (ResponsePattern){
        .keywords = malloc(4 * sizeof(char*)),
        .keyword_count = 3,
        .response = strdup("Your tenant dashboard shows: 1) Rent payment status 2) Recent maintenance requests 3) Property announcements 4) Upcoming lease dates. Available buttons: [Home, Payments, Requests, Profile]"),
        .category = strdup("dashboard"),
        .user_role = strdup("tenant"),
        .language = strdup("en"),
        .confidence_threshold = 0.8f
    };
    patterns[count].keywords[0] = strdup("dashboard");
    patterns[count].keywords[1] = strdup("home");
    patterns[count].keywords[2] = strdup("main page");
    count++;

    return patterns;
}

void load_tenant_routes(void) {
    static char *tenant_nav_buttons[] = {"Home", "Payments", "Requests", "Profile"};
    static char *dashboard_actions[] = {"View Dashboard", "Check Notifications", "Recent Activity"};
    static char *payment_actions[] = {"Pay Rent", "View History", "Payment Methods", "Due Dates"};
    static char *request_actions[] = {"New Request", "View Status", "Request History"};
    static char *profile_actions[] = {"Edit Profile", "Change Password", "Contact Info", "Documents"};

    tenant_route_count = 4;
    tenant_route_guides = malloc(tenant_route_count * sizeof(RouteGuide));
    if (!tenant_route_guides) {
        log_message("ERROR", "Failed to allocate memory for tenant routes");
        return;
    }

    tenant_route_guides[0].route = strdup("/tenant");
    tenant_route_guides[0].user_role = strdup("tenant");
    tenant_route_guides[0].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[0].page_actions = dashboard_actions;
    tenant_route_guides[0].patterns = NULL;
    tenant_route_guides[0].pattern_count = 2;

    tenant_route_guides[1].route = strdup("/tenant/payments");
    tenant_route_guides[1].user_role = strdup("tenant");
    tenant_route_guides[1].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[1].page_actions = payment_actions;
    tenant_route_guides[1].patterns = NULL;
    tenant_route_guides[1].pattern_count = 3;

    tenant_route_guides[2].route = strdup("/tenant/requests");
    tenant_route_guides[2].user_role = strdup("tenant");
    tenant_route_guides[2].navigation_buttons = tenant_nav_buttons;
    tenant_route_guides[2].page_actions = request_actions;
    tenant_route_guides[2].patterns = NULL;
    tenant_route_guides[2].pattern_count = 2;

    tenant_route_guides[3].route = strdup("/tenant/profile");
    tenant_route_guides[3].user_role = strdup("tenant");
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