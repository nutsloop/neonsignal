# Language Design Philosophy

## Overview

A custom language with LLVM JIT compilation for NeonSignal - designed for full application framework capabilities with rich C++ bindings and memory efficiency.

## Language Design Philosophy

Given NeonSignal's HTTP/2 + async I/O architecture, the language should be:

1. **Async-first** - All I/O is non-blocking by default
2. **Strongly typed** - Catch errors at compile time, enable LLVM optimizations
3. **Zero-cost abstractions** - High-level syntax, low-level performance
4. **Memory safe** - No manual memory management, but predictable allocation

---

## Language Syntax Design

### Basic Handler Structure

```typescript
// handlers/users.ns (NeonEcho)

import { db, cache } from "neonsignal:runtime";
import { User, UserResponse } from "./types";

@route("GET", "/api/users/:id")
async fn get_user(req: Request) -> Response {
    let user_id = req.params.id.parse::<u64>()?;

    // Check cache first
    if let Some(cached) = await cache.get(f"user:{user_id}") {
        return Response.json(cached).with_header("X-Cache", "HIT");
    }

    // Query database
    let user = await db.query_one::<User>(
        "SELECT id, name, email FROM users WHERE id = $1",
        [user_id]
    )?;

    match user {
        Some(u) => {
            await cache.set(f"user:{user_id}", u, ttl: 300);
            Response.json(u)
        }
        None => Response.error("User not found", 404)
    }
}

@route("POST", "/api/users")
async fn create_user(req: Request) -> Response {
    let body = req.json::<CreateUserRequest>()?;

    // Validation
    if body.email.len() < 5 || !body.email.contains('@') {
        return Response.error("Invalid email", 400);
    }

    let user_id = await db.execute(
        "INSERT INTO users (name, email) VALUES ($1, $2) RETURNING id",
        [body.name, body.email]
    )?;

    Response.json({ id: user_id, ...body }).with_status(201)
}
```

### Type System

```typescript
// types.ns
struct User {
    id: u64,
    name: String,
    email: String,
    created_at: Timestamp
}

struct CreateUserRequest {
    name: String,
    email: String
}

enum ApiError {
    NotFound(String),
    ValidationError(String),
    DatabaseError(String)
}

// Compile-time type checking
impl From<DbError> for ApiError {
    fn from(err: DbError) -> ApiError {
        ApiError::DatabaseError(err.message)
    }
}
```

### Async/Await Model

```typescript
// Async is based on continuations, compiles to state machine

async fn complex_handler(req: Request) -> Response {
    // Each await point becomes a suspension point
    let user = await fetch_user(req.params.id);
    let posts = await fetch_posts(user.id);
    let comments = await fetch_comments(posts.map(|p| p.id));

    // Compiler generates state machine:
    // State0: fetch_user -> State1
    // State1: fetch_posts -> State2
    // State2: fetch_comments -> State3
    // State3: build response

    Response.json({ user, posts, comments })
}
```

---

## LLVM JIT Integration Architecture

### Compilation Pipeline

```
Source Code (.ns file)
    ↓
┌─────────────────────┐
│   Lexer/Parser      │  ← Use ANTLR4 or hand-written recursive descent
│   (C++)             │
└──────────┬──────────┘
           ↓
      AST (Abstract Syntax Tree)
           ↓
┌─────────────────────┐
│   Type Checker      │  ← Hindley-Milner type inference
│   (C++)             │
└──────────┬──────────┘
           ↓
      Typed AST
           ↓
┌─────────────────────┐
│   IR Generator      │  ← Lower to LLVM IR
│   (LLVM Builder)    │
└──────────┬──────────┘
           ↓
      LLVM IR Module
           ↓
┌─────────────────────┐
│   Optimization      │  ← LLVM optimization passes
│   (LLVM PassManager)│
└──────────┬──────────┘
           ↓
      Optimized IR
           ↓
┌─────────────────────┐
│   JIT Compilation   │  ← ORC JIT v2
│   (LLJIT)           │
└──────────┬──────────┘
           ↓
      Native Machine Code
           ↓
┌─────────────────────┐
│   Execute           │
└─────────────────────┘
```

### LLVM Integration Code Structure

```cpp
// script_compiler.h++
#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <memory>
#include <string_view>

namespace neonsignal::scripting {

class ScriptCompiler {
public:
    ScriptCompiler();

    // Compile source to LLVM module
    std::unique_ptr<llvm::Module> compile(
        std::string_view source_code,
        std::string_view file_name
    );

    // JIT compile and get function pointer
    using HandlerFn = void* (*)(void* request_ctx);
    HandlerFn get_handler(const std::string& function_name);

private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::orc::LLJIT> jit_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    // AST -> IR generation
    llvm::Value* codegen_expr(const ASTNode& node);
    llvm::Function* codegen_function(const FunctionDecl& fn);
    llvm::Value* codegen_await(const AwaitExpr& expr);
};

} // namespace neonsignal::scripting
```

### C++ Runtime Bindings

```cpp
// script_runtime.h++
#pragma once

#include <string_view>
#include <cstdint>

namespace neonsignal::scripting::runtime {

// These functions are called from JIT-compiled code
// Must use C ABI for LLVM external function calls

extern "C" {

// Memory allocation (GC-managed)
void* ns_alloc(uint64_t size);
void ns_dealloc(void* ptr);

// String operations
void* ns_string_new(const char* data, uint64_t len);
uint64_t ns_string_len(void* str);
const char* ns_string_data(void* str);

// Async operations (returns Future handle)
void* ns_async_db_query(const char* sql, void* params);
void* ns_async_http_fetch(const char* url);
void* ns_async_cache_get(const char* key);

// Future operations
void* ns_future_await(void* future); // Suspends coroutine
bool ns_future_is_ready(void* future);

// Response building
void* ns_response_new();
void ns_response_set_status(void* resp, uint32_t status);
void ns_response_set_body(void* resp, const char* body, uint64_t len);
void ns_response_set_header(void* resp, const char* name, const char* value);

} // extern "C"

} // namespace neonsignal::scripting::runtime
```

---

## Async/Await Implementation Strategy

This is the most complex part. Here's how to compile async to LLVM IR:

### Coroutine Transformation

```typescript
// Source code:
async fn handler(req: Request) -> Response {
    let user = await db.query(...);
    let posts = await db.query(...);
    Response.json({ user, posts })
}

// Compiles to state machine (conceptual):
struct HandlerState {
    state: u32,
    req: Request,
    user: Option<User>,
    posts: Option<Vec<Post>>
}

fn handler_resume(state: *mut HandlerState) -> ResumeResult {
    match state.state {
        0 => {
            // Start db.query, return Pending
            let future = ns_async_db_query(...);
            state.state = 1;
            return ResumeResult::Pending(future);
        }
        1 => {
            // user query completed
            state.user = Some(get_future_result(...));
            let future = ns_async_db_query(...);
            state.state = 2;
            return ResumeResult::Pending(future);
        }
        2 => {
            // posts query completed
            state.posts = Some(get_future_result(...));
            let response = build_response(state.user, state.posts);
            return ResumeResult::Ready(response);
        }
    }
}
```

### LLVM IR for Async (simplified)

```llvm
; Coroutine state structure
%HandlerState = type {
    i32,           ; state
    %Request*,     ; request
    %User*,        ; user (nullable)
    %PostVec*      ; posts (nullable)
}

define %ResumeResult @handler_resume(%HandlerState* %state) {
entry:
    %state_val = load i32, i32* %state
    switch i32 %state_val, label %unreachable [
        i32 0, label %state0
        i32 1, label %state1
        i32 2, label %state2
    ]

state0:
    ; Call runtime function for async db query
    %future0 = call i8* @ns_async_db_query(i8* %sql, i8* %params)

    ; Update state
    store i32 1, i32* %state

    ; Return pending
    %result0 = insertvalue %ResumeResult undef, i32 0, 0  ; Pending
    %result0_1 = insertvalue %ResumeResult %result0, i8* %future0, 1
    ret %ResumeResult %result0_1

state1:
    ; Get result from completed future
    %user = call i8* @ns_future_get_result(i8* %future0)
    store i8* %user, i8** %state.user

    ; Start next query
    %future1 = call i8* @ns_async_db_query(...)
    store i32 2, i32* %state

    %result1 = insertvalue %ResumeResult undef, i32 0, 0
    %result1_1 = insertvalue %ResumeResult %result1, i8* %future1, 1
    ret %ResumeResult %result1_1

state2:
    ; All data ready, build response
    %user_ptr = load i8*, i8** %state.user
    %posts_ptr = load i8*, i8** %state.posts
    %response = call i8* @build_json_response(%user_ptr, %posts_ptr)

    %result2 = insertvalue %ResumeResult undef, i32 1, 0  ; Ready
    %result2_1 = insertvalue %ResumeResult %result2, i8* %response, 1
    ret %ResumeResult %result2_1

unreachable:
    unreachable
}
```

---

## Integration with Event Loop

The event loop needs to drive coroutine execution:

```cpp
// In your http2_listener/handle_io_.c++

void Http2Listener::execute_script_handler(
    std::shared_ptr<Http2Connection> conn,
    const std::string& handler_name,
    const Request& req
) {
    // Get compiled handler
    auto handler_fn = compiler_->get_handler(handler_name);

    // Create coroutine state
    auto* coro_state = ns_coroutine_create(handler_fn, &req);

    // Initial resume
    auto result = ns_coroutine_resume(coro_state);

    while (result.status == ResumeStatus::Pending) {
        // Register future with event loop
        auto* future = result.future;

        // When future's FD becomes ready, event loop calls:
        event_loop_->register_callback(future->fd, EPOLLIN, [=](uint32_t events) {
            // Mark future as ready
            ns_future_complete(future);

            // Resume coroutine
            auto next_result = ns_coroutine_resume(coro_state);

            if (next_result.status == ResumeStatus::Ready) {
                // Send response
                send_response(conn, next_result.response);
                ns_coroutine_destroy(coro_state);
            } else {
                // More async work, repeat
            }
        });

        return; // Yield to event loop
    }

    // Handler completed synchronously
    send_response(conn, result.response);
    ns_coroutine_destroy(coro_state);
}
```

---

## Memory Management Strategy

**Option A: Reference Counting** (simpler, predictable)
```llvm
; Every heap object has ref count
%Object = type {
    i32,    ; ref_count
    i8*     ; data
}

; Generated for every assignment
define void @retain(%Object* %obj) {
    %rc = getelementptr %Object, %Object* %obj, i32 0, i32 0
    %old = atomicrmw add i32* %rc, i32 1 seq_cst
    ret void
}

define void @release(%Object* %obj) {
    %rc = getelementptr %Object, %Object* %obj, i32 0, i32 0
    %old = atomicrmw sub i32* %rc, i32 1 seq_cst
    %is_zero = icmp eq i32 %old, 1
    br i1 %is_zero, label %dealloc, label %done

dealloc:
    call void @ns_dealloc(%Object* %obj)
    br label %done

done:
    ret void
}
```

**Option B: Arena Allocator** (faster, per-request lifetime)
```cpp
// All allocations during request handled in arena
// Freed all at once when request completes

struct RequestArena {
    std::vector<std::byte*> chunks;
    size_t current_offset = 0;

    void* allocate(size_t size) {
        // Bump allocator logic
    }

    ~RequestArena() {
        // Free all chunks at once
        for (auto* chunk : chunks) {
            std::free(chunk);
        }
    }
};
```

**Recommendation:** Hybrid approach
- Per-request arena for short-lived allocations
- Reference counting for objects that outlive request (cached data)

---

## Optimization Opportunities

LLVM enables powerful optimizations:

### 1. **Inlining**
```typescript
// Source
fn parse_id(s: String) -> u64 {
    s.parse::<u64>().unwrap_or(0)
}

fn handler(req: Request) -> Response {
    let id = parse_id(req.params.id);
    // ...
}

// LLVM can inline parse_id directly into handler
// Zero function call overhead
```

### 2. **Constant Folding**
```typescript
const MAX_PAGE_SIZE: u64 = 100;

fn get_page_size(req: Request) -> u64 {
    min(req.query.limit.parse(), MAX_PAGE_SIZE)
}

// LLVM can precompute bounds checks
```

### 3. **Dead Code Elimination**
```typescript
if (DEBUG_MODE) {
    log("Debug info");
}

// If DEBUG_MODE = false, entire branch removed at compile time
```

### 4. **Type-Based Alias Analysis**
```typescript
// LLVM knows these don't alias, can reorder/optimize aggressively
let user: User = ...;
let post: Post = ...;
```

---

## Development Roadmap

### **Phase 1: Minimal Viable Language (3-4 months)**
- Lexer/Parser for basic syntax (functions, let, literals, function calls)
- Simple type checker (no inference yet, explicit types only)
- LLVM IR codegen for basic operations
- JIT compilation and execution
- C++ FFI for request/response
- **Deliverable:** Synchronous handlers that can read request, return response

### **Phase 2: Type System (2-3 months)**
- Structs, enums, pattern matching
- Type inference (Hindley-Milner)
- Generics/parametric polymorphism
- **Deliverable:** Rich type system, compile-time safety

### **Phase 3: Async/Await (4-5 months)**
- Coroutine transformation
- Future/Promise runtime
- Event loop integration
- **Deliverable:** Non-blocking I/O handlers

### **Phase 4: Standard Library (2-3 months)**
- String manipulation, JSON parsing
- HTTP client, database bindings
- Logging, caching
- **Deliverable:** Production-ready runtime

### **Phase 5: Tooling (ongoing)**
- LSP server (IDE support)
- Debugger integration
- Hot reload
- **Deliverable:** Developer experience

**Total timeline: ~12-15 months for production-ready system**

---

## Immediate Next Steps

Priority options:

1. **Design the grammar formally** (EBNF notation for the language)
2. **Prototype the lexer/parser** (initial implementation)
3. **Set up LLVM build infrastructure** (integrate into Meson build)
4. **Design the runtime API** (finalize the C++ interface that scripts call)
5. **Create proof-of-concept** (hello world handler compiled to LLVM IR)
