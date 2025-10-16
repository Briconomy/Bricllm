# Bricllm

**High-Performance Lightweight Chatbot for Briconomy PWA**

Bricllm is a fast, efficient "fake" LLM chatbot designed for the Briconomy Progressive Web Application. Built in C for maximum performance, it provides natural conversational assistance for navigating the Briconomy app features.

## Features

- **Conversational AI** - Natural language interaction with varied responses  
- **High Performance** - Sub-50ms response times, <20MB memory footprint  
- **Context-Aware** - Understands user roles (Tenant, Caretaker, Manager, Admin)  
- **Multi-Language** - Supports English and Zulu  
- **Navigation Assistant** - Helps users navigate app routes and features  
- **Smart Deflection** - Handles small talk and redirects to app assistance  

## Quick Start

### Build
```bash
make
```

### Run
```bash
./bricllm
```

### Clean Build Artifacts
```bash
make clean
```

## Usage

### Commands
```
/help                 - Show help message
/clear                - Clear screen
/role <role>          - Set user role (tenant, caretaker, manager, admin)
/lang <lang>          - Set language (en, zu)
/route <path>         - Set current route context
/status               - Show current session status
/quit                 - Exit the application
```

### Natural Language Examples
```
"How do I pay my rent?"
"Where can I find maintenance requests?"
"Show me the navigation buttons"
"What can I do on this page?"
```

### Conversational Queries
Bricllm handles casual conversation naturally:
- Greetings: "hello", "hi", "hey"
- Small talk: "how are you?"
- Identity questions: "are you a bot?"
- Off-topic: "what's the weather?"

All queries are politely redirected to app assistance while maintaining a natural conversational tone.

## Project Structure

```
bricllm/
├── src/
│   ├── core/
│   │   ├── chat_engine.c          # Main chat processing logic
│   │   └── pattern_matcher.c      # Keyword and fuzzy matching
│   ├── data/
│   │   └── route_system.c         # Route and navigation handling
│   ├── routes/
│   │   └── tenant_routes.c        # Tenant-specific navigation
│   └── utils/
│       └── logger.c               # Debug logging system
├── include/
│   ├── bricllm.h                  # Main header file
│   ├── chat_engine.h              # Chat engine interfaces
│   └── route_types.h              # Route data structures
├── Makefile                       # Build configuration
└── main.c                         # Entry point
```

## Technical Details

### Performance Targets
- **Response Time**: < 100ms for cached queries, < 500ms for complex matches
- **Memory Usage**: < 50MB steady state
- **Concurrent Users**: 1000+ simultaneous sessions
- **CPU Usage**: < 10% under normal load

### Pattern Matching
Uses fuzzy string matching with Levenshtein distance algorithm to understand user intent even with typos or variations in phrasing.

### Response Strategy
- Multiple response variations per category (3+ options)
- Random selection prevents robotic repetition
- Context-aware based on user role and current app route
- Graceful fallback for unrecognized queries

## Supported User Roles

### Tenant
- Navigate: [Home, Payments, Requests, Profile]
- Features: Rent payments, maintenance requests, lease information

### Caretaker  
- Navigate: [Tasks, Schedule, History, Profile]
- Features: Task management, scheduling, work history

### Manager
- Navigate: [Dashboard, Properties, Leases, Payments]
- Features: Property management, lease administration, financial reports

### Admin
- Navigate: [Dashboard, Users, Security, Reports]
- Features: User management, system administration, security settings

## Development

### Requirements
- GCC (C11 standard or later)
- Make
- Linux/Unix environment

### Building
```bash
# Standard build
make

# Clean build
make clean && make

# Debug build (with extra logging)
make DEBUG=1
```

### Testing
```bash
# Run test script
./test.sh

# Interactive testing
./bricllm
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Follow the existing code style
2. Add tests for new features
3. Update documentation as needed
4. Ensure all builds pass without errors

## Documentation

- [USAGE.md](USAGE.md) - Detailed usage instructions
- [IMPROVEMENTS.md](IMPROVEMENTS.md) - Recent conversational improvements
- [LayoutAndStructureGuidelines.md](LayoutAndStructureGuidelines.md) - UI layout guidelines
- [UserFacingRoutesAndLayouts.md](UserFacingRoutesAndLayouts.md) - Route documentation

## License

See LICENSE file for details.

## About Briconomy

Bricllm is part of the Briconomy ecosystem - a comprehensive property management platform for tenants, caretakers, managers, and administrators.

---

**Built in C for maximum performance**
