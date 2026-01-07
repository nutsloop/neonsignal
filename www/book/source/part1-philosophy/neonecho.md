# The Custom Scripting Language

```{epigraph}
"NeonEcho isn't just a language—it's a statement. In a world of JavaScript fatigue and framework churn, we're designing a language that treats the server as a first-class citizen, not an afterthought."

— Language Design Document
```

```{admonition} Future Vision
:class: tip

NeonEcho is an **aspirational design**—a vision for where NeonSignal could go. The language specification below documents the intended syntax and semantics, but **no implementation currently exists**. This is a long-term roadmap item that would require significant development effort (LLVM integration, parser, type system, etc.).
```

NeonEcho is a custom-designed language with LLVM JIT compilation capabilities, created to provide a full application framework within NeonSignal. It is designed for memory efficiency, async-first I/O, and seamless C++ interoperability.

---

## Why a New Language?

Before diving into syntax, let's address the obvious question: *Why not use an existing language?*

```{list-table}
:header-rows: 1
:widths: 20 80

* - Alternative
  - Why Not
* - **Rust**
  - Borrow checker adds ceremony for web handler patterns. Lifetime annotations clutter simple request/response code.
* - **Go**
  - Garbage collector causes latency spikes in real-time SSE scenarios. No generics (until recently), limited type expressiveness.
* - **TypeScript**
  - V8 runtime overhead. No native HTTP/2 integration. JIT warmup latency.
* - **C++**
  - Too verbose for rapid handler development. Template metaprogramming is powerful but obscure.
```

### The NeonEcho Sweet Spot

NeonEcho aims to combine the best of each:

- **Rust-like safety** without borrow checker ceremony
- **Go-like simplicity** without GC pauses
- **TypeScript-like ergonomics** with native compilation
- **C++ performance** with modern, readable syntax

---

## Language Design Philosophy

Given NeonSignal's HTTP/2 + async I/O architecture, NeonEcho is designed around four pillars:

```{list-table}
:header-rows: 1
:widths: 25 75

* - Principle
  - Implementation
* - **Async-First**
  - All I/O is non-blocking by default. The `turbo` keyword marks async functions; `ride` awaits them. No callback hell, no promise chains.
* - **Strongly Typed**
  - Catches errors at compile time. Enables LLVM optimizations. No `any` type, no runtime type checks.
* - **Zero-Cost Abstractions**
  - High-level syntax compiles to low-level performance. `entity` becomes a C struct. `arcade` becomes a jump table.
* - **Memory Safe**
  - No manual memory management. Automatic Reference Counting (ARC) with cycle detection. Predictable deallocation at scope exit.
```

---

## Radical Syntax Showcase

NeonEcho is designed to be visually expressive and fun to write. The synthwave aesthetic extends from NeonSignal's visual design into the language itself.

### Basic Wave Handlers

**Simple GET Request:**

```text
@route("GET", "/api/hello")
wave greet_the_world(req: Request) ~> Response {
    wire name = req.query.get("name") <?> "Stranger";

    bounce Response.json({
        message: neon"Hello, {name}! Welcome to the Grid",
        timestamp: now(),
        vibes: "rad"
    })
}
```

**With Route Parameters:**

```text
@route("GET", "/api/riders/:id")
wave fetch_rider(req: Request) ~> Response {
    wire rider_id = req.params.id.parse::<u64>()?;
    wire rider = database.find_rider(rider_id);

    arcade rider {
        jackpot(r) => Response.json(r),
        void => Response.error("Rider vanished from the Grid", 404)
    }
}
```

### Turbo Mode (Async)

**Async Database Query:**

```text
@route("GET", "/api/users/:id")
@cache(300)  /// Cache for 5 minutes
turbo wave fetch_user(req: Request) ~> Response {
    wire user_id = req.params.id.parse::<u64>()?;

    /// Ride the async wave through the database
    wire user = ride db.query_one::<User>(
        "SELECT id, name, email, bike FROM users WHERE id = $1",
        [user_id]
    )?;

    arcade user {
        jackpot(u) => {
            ride cache.set(neon"user:{user_id}", u, ttl: 300);
            Response.json(u).with_header("X-Cache", "MISS")
        }
        void => Response.error("User not found in the Grid", 404)
    }
}
```

### Error Handling

**Using `bail` for Early Returns:**

```text
turbo wave process_order(req: Request) ~> Response {
    wire user = ride auth.validate(req.session)?;
    wire order = ride db.create_order(user.id)?;

    portal order.total > 1000 {
        ride notifications.alert_fraud_team(order);
        bail Response.error("Order flagged for review", 202)
    }

    bounce Response.json(order)
}
```

**The `?` Operator:**

```text
/// Propagates errors up the call stack
wire config = read_config()?;          /// Returns early if error
wire parsed = config.parse::<Settings>()?;
wire validated = parsed.validate()?;
```

### SSE Streaming

**Real-Time Metrics Stream:**

```text
@route("GET", "/api/metrics/stream")
turbo wave stream_metrics(req: Request) ~> SSEStream {
    wire flux event_count = 0;

    loop {
        wire metrics = ride system.collect_metrics();
        event_count += 1;

        yield SSEEvent {
            event: "metrics",
            data: metrics.to_json(),
            id: neon"{event_count}"
        };

        ride sleep(16ms);  /// ~60fps batching
    }
}
```

---

## Entities and Variants

### Entity Declaration (Struct)

```text
entity Rider {
    broadcast id: u64,
    broadcast name: Text,
    broadcast bike: Text,
    broadcast power_level: u32,
    whisper secret_ability: Text,    /// Private field
    broadcast created_at: Timestamp
}
```

- `broadcast` = public field (visible externally)
- `whisper` = private field (internal only)

### Variant Declaration (Enum)

```text
variant GameResult {
    Victory(u64),                    /// Associated score
    Defeat(Text),                    /// Reason for defeat
    TimeOut,                         /// No associated data
    Continue {                       /// Struct variant
        level: u32,
        lives: u8,
        boss_health: f32
    }
}
```

### Protocol (Interface) Definition

```text
protocol Cacheable {
    wave cache_key(self) ~> Text;
    wave ttl(self) ~> Duration;
    wave serialize(self) ~> Bytes;
    static wave deserialize(data: Bytes) ~> Result<Self>;
}

entity User powers Cacheable {
    broadcast id: u64,
    broadcast email: Text,
    broadcast display_name: Text,

    wave cache_key(self) ~> Text {
        bounce neon"user:{self.id}"
    }

    wave ttl(self) ~> Duration {
        bounce 5.minutes()
    }

    wave serialize(self) ~> Bytes {
        bounce json.encode(self)
    }

    static wave deserialize(data: Bytes) ~> Result<Self> {
        bounce json.decode::<User>(data)
    }
}
```

---

## Type System

### Primitive Types

```text
/// Integers (unsigned)
wire small: u8 = 255;
wire medium: u32 = 4_294_967_295;
wire big: u64 = 18_446_744_073_709_551_615;

/// Integers (signed)
wire negative: i32 = -42;
wire large_signed: i64 = -9_223_372_036_854_775_808;

/// Floating point
wire precise: f64 = 3.14159265359;
wire fast: f32 = 3.14;

/// Text (UTF-8, heap allocated)
wire name: Text = "NeonSignal";
wire interpolated: Text = neon"Server: {name}";

/// Bytes (raw binary data)
wire data: Bytes = b"\x00\x01\x02\xff";

/// Boolean
wire active: bool = rad;      /// true
wire disabled: bool = bogus;  /// false
```

### Optional Types

```text
/// No null in NeonEcho - use Option<T>
wire maybe_user: Option<User> = jackpot(user);  /// Some
wire no_user: Option<User> = void;              /// None

/// Unwrap with default
wire name = maybe_user.map(|u| u.name) <?> "Anonymous";

/// Pattern matching
arcade maybe_user {
    jackpot(u) => print(neon"Found: {u.name}"),
    void => print("No user found")
}
```

### Result Types

```text
/// Explicit error handling - no exceptions
wire result: Result<User, DbError> = jackpot(user);   /// Ok
wire error: Result<User, DbError> = glitch(DbError::NotFound);  /// Err

/// Propagate with ?
wire user = db.find_user(id)?;  /// Returns early if glitch

/// Handle explicitly
arcade db.find_user(id) {
    jackpot(u) => process_user(u),
    glitch(DbError::NotFound) => Response.error("Not found", 404),
    glitch(DbError::Connection(msg)) => Response.error(msg, 503),
    glitch(e) => Response.error(neon"Unknown: {e}", 500)
}
```

### Collection Types

```text
/// Array (fixed size, stack allocated)
wire coords: [f64; 3] = [1.0, 2.0, 3.0];
wire matrix: [[i32; 3]; 3] = [
    [1, 0, 0],
    [0, 1, 0],
    [0, 0, 1]
];

/// Vector (dynamic, heap allocated)
wire items: Vec<Text> = ["alpha", "beta", "gamma"];
items.push("delta");
wire first = items[0];  /// "alpha"

/// HashMap
wire scores: Map<Text, u64> = {
    "player1": 100,
    "player2": 250,
    "player3": 175
};
wire p1_score = scores.get("player1") <?> 0;

/// HashSet
wire tags: Set<Text> = {"async", "http2", "tls"};
portal tags.contains("http2") {
    print("HTTP/2 enabled");
}
```

---

## Memory Model

### Ownership Without Borrow Checker

NeonEcho uses a simpler ownership model than Rust, trading some safety guarantees for ergonomics.

```text
/// Values are moved by default
wire original: Text = "hello";
wire moved = original;
/// original is now invalid - compile error to use it

/// Clone for explicit copy
wire original: Text = "hello";
wire copied = original.clone();
/// Both original and copied are valid

/// References for borrowing
wave process(data: &Text) {
    /// Can read data, cannot modify
    print(data);
}

wave mutate(data: &flux Text) {
    /// Can read and modify
    data.push_str(" world");
}
```

### Automatic Reference Counting (ARC)

```text
/// No manual memory management
/// No garbage collector pauses
/// Predictable deallocation at scope exit

entity Connection {
    broadcast socket: TcpSocket,
    broadcast buffer: Vec<u8>
}

wave handle_connection(conn: Connection) {
    /// conn is owned by this function
    process(conn);
    /// conn is deallocated here (refcount -> 0)
}

/// Shared ownership with Arc<T>
wire shared: Arc<Config> = Arc::new(load_config());
wire clone1 = shared.clone();  /// refcount: 2
wire clone2 = shared.clone();  /// refcount: 3
/// All clones share the same data
/// Deallocated when last reference drops
```

---

## Concurrency Model

### Async Runtime Integration

NeonEcho's async runtime integrates directly with NeonSignal's epoll event loop.

```text
/// Turbo functions run on the event loop
turbo wave fetch_data(url: Text) ~> Result<Data, HttpError> {
    wire response = ride http.get(url)?;
    bounce response.json::<Data>()
}

/// Parallel execution
turbo wave fetch_all_users(ids: Vec<u64>) ~> Vec<User> {
    wire futures = ids.map(|id| fetch_user(id));
    bounce ride parallel(futures)
}

/// Racing (first to complete wins)
turbo wave fetch_with_fallback() ~> Data {
    bounce ride race([
        fetch_from_primary(),
        fetch_from_backup()
    ])
}
```

### Channel-Based Communication

```text
/// Create a channel
wire (tx, rx) = channel::<Event>(100);  /// Buffer size: 100

/// Producer
turbo wave event_producer(tx: Sender<Event>) {
    loop {
        wire event = ride generate_event();
        ride tx.send(event)?;
    }
}

/// Consumer
turbo wave event_consumer(rx: Receiver<Event>) {
    chase event in rx {
        process_event(event);
    }
}

/// Select from multiple channels
turbo wave multiplexer(rx1: Receiver<A>, rx2: Receiver<B>) {
    loop {
        select {
            a from rx1 => handle_a(a),
            b from rx2 => handle_b(b),
            timeout(1.second()) => handle_timeout()
        }
    }
}
```

---

## Module System

```text
/// Declare module (file: auth.ne)
cassette auth;

/// Import from standard library
plug std::crypto::sha256;
plug std::time::{Duration, Instant};

/// Import from NeonSignal runtime
plug neonsignal::http::{Request, Response};
plug neonsignal::sse::SSEStream;

/// Import from local crate
plug crate::database::User;
plug crate::config::Settings;

/// Re-export for external use
beam greet_handler;
beam verify_token;
beam AuthError;

/// Private to module (not exported)
whisper internal_helper;
whisper validate_internal;
```

**Using Modules:**

```text
/// In another file
plug auth::{greet_handler, verify_token};
plug auth::AuthError;

/// Qualified access
wire result = auth::verify_token(token)?;
```

---

## C++ Interoperability

### FFI Bridge

```text
/// Import C++ function
extern "C++" {
    /// From OpenSSL
    wave openssl_sha256(data: &Bytes) ~> [u8; 32];

    /// From nghttp2
    wave nghttp2_submit_response(
        session: *mut NgHttp2Session,
        stream_id: i32,
        headers: &Headers
    ) ~> i32;

    /// From NeonSignal
    wave neonsignal_get_connection(id: u64) ~> *mut Connection;
}

/// Export NeonEcho function to C++
@export("handle_api_request")
wave handle_request(req: *const Request) ~> *mut Response {
    wire safe_req = Request::from_ptr(req)?;
    wire response = process_request(safe_req);
    bounce response.into_ptr()
}
```

### Struct Layout Compatibility

```text
/// C-compatible struct layout
@repr(C)
entity HttpHeader {
    broadcast name: *const u8,
    broadcast name_len: usize,
    broadcast value: *const u8,
    broadcast value_len: usize
}

/// Use in FFI calls
wire headers: Vec<HttpHeader> = build_headers();
nghttp2_submit_response(session, stream_id, headers.as_ptr());
```

---

## LLVM Compilation Pipeline

```
                    NeonEcho Source (.ne)
                            │
                            ▼
              ┌─────────────────────────────┐
              │       Lexer / Parser        │
              │   (Custom, Rust or C++)     │
              └─────────────┬───────────────┘
                            │ AST
                            ▼
              ┌─────────────────────────────┐
              │       Type Checker          │
              │  (Hindley-Milner + traits)  │
              └─────────────┬───────────────┘
                            │ Typed AST
                            ▼
              ┌─────────────────────────────┐
              │      IR Generator           │
              │   (NeonEcho IR → LLVM IR)   │
              └─────────────┬───────────────┘
                            │ LLVM IR
                            ▼
              ┌─────────────────────────────┐
              │       LLVM Backend          │
              │   (Optimization passes)     │
              │   -O2, inlining, vectorize  │
              └─────────────┬───────────────┘
                            │
                            ▼
                     Native Binary
                 (or JIT for development)
```

---

## Grammar Specification

The following is a detailed EBNF-style grammar for NeonEcho, presented in its "Radical-BNF" dialect.

**Notation:**
- `~>` Definition
- `|` Alternative
- `{}` Zero or more
- `[]` Zero or one
- `<>` One or more

### Keywords

```text
keyword ~>
    /* Flow Control */
    | "wave" | "turbo" | "ride" | "wire" | "static" | "flux"

    /* Control Structures */
    | "portal" | "otherwise" | "arcade" | "bounce" | "bail" | "cruise"

    /* Loops */
    | "loop" | "while" | "chase"

    /* Type Definitions */
    | "entity" | "variant" | "powers" | "protocol"

    /* Module System */
    | "plug" | "beam" | "cassette" | "whisper" | "broadcast"

    /* Concurrency */
    | "select" | "timeout" | "parallel" | "race"

    /* Literals */
    | "rad" | "bogus" | "void" | "jackpot" | "glitch"
```

### Expressions and Statements

```text
statement ~>
    | expression_pulse
    | wire_statement
    | portal_branch
    | arcade_match
    | loop_statement
    | bounce_statement
    | bail_statement

expression_pulse ~> expression ";"

wire_statement ~>
    "wire"
    ["flux"]
    pattern
    [type_mark]
    ["=" expression]
    ";"

block_portal ~> "{" {statement} [expression] "}"

portal_branch ~>
    "portal" expression block_portal
    ["otherwise" (portal_branch | block_portal)]

arcade_match ~>
    "arcade" expression "{"
        [arcade_round {"," arcade_round} [","]]
    "}"

arcade_round ~>
    pattern ["portal" expression] "=>" (expression | block_portal)

loop_statement ~>
    | "loop" block_portal
    | "while" expression block_portal
    | "chase" pattern "in" expression block_portal

bounce_statement ~> "bounce" expression

bail_statement ~> "bail" expression
```

For the complete grammar specification, see [NeonEcho_Grammar.md](https://github.com/nutsloop/neonsignal/blob/main/plans/NeonEcho_Grammar.md).

---

## Development Roadmap

```{admonition} Implementation Status
:class: warning

This roadmap represents the aspirational development plan. No implementation work has begun.
```

### Phase 1: Foundation

- [ ] Lexer and parser implementation (Rust or C++)
- [ ] Basic type system (primitives, structs, enums)
- [ ] LLVM IR generation for simple programs
- [ ] Basic standard library (I/O, strings, collections)
- [ ] Error messages with source locations

### Phase 2: Async Runtime

- [ ] Event loop integration with NeonSignal's epoll
- [ ] `turbo`/`ride` async syntax implementation
- [ ] Channel implementation for concurrency
- [ ] Timer and sleep primitives
- [ ] Select/race combinators

### Phase 3: Web Primitives

- [ ] HTTP/2 Request/Response types
- [ ] SSE stream support with yield syntax
- [ ] WebSocket support
- [ ] JSON serialization/deserialization
- [ ] Route attribute macros

### Phase 4: Production Ready

- [ ] Full C++ FFI interoperability
- [ ] Debug symbols and source maps
- [ ] Error messages with fix suggestions
- [ ] Package manager (`cassette` system)
- [ ] Documentation generator
- [ ] Language server protocol (LSP) for IDE support

---

## Why "Radical Syntax"?

The synthwave aesthetic isn't just visual decoration—it's philosophical. Every keyword choice reflects a mindset.

```{list-table}
:header-rows: 1
:widths: 20 20 60

* - NeonEcho
  - Traditional
  - Philosophy
* - `wave`
  - `fn` / `func`
  - Functions flow like radio waves through the system
* - `turbo`
  - `async`
  - Speed is the default mindset, not an afterthought
* - `ride`
  - `await`
  - Surfing the async wave, not blocking on it
* - `arcade`
  - `match`
  - Pattern matching as a game—find the winning pattern
* - `jackpot`
  - `Some`
  - Finding what you're looking for is a win
* - `void`
  - `None`
  - Absence is the void between the neon lights
* - `glitch`
  - `Err`
  - Errors are glitches in the matrix
* - `portal`
  - `if`
  - Conditionals are portals to different code paths
* - `bounce`
  - `return`
  - Values bounce back to the caller
* - `entity`
  - `struct`
  - Data structures are entities in the digital world
* - `cassette`
  - `mod`
  - Modules are cassettes you plug into the system
```

This isn't syntax sugar for its own sake. It's a reminder that:

- **Coding should be fun.** If we're going to spend our lives writing code, it should bring joy.
- **Performance is exhilarating.** There's beauty in a perfectly optimized hot path.
- **Building the future is an adventure.** We're not just writing CRUD apps—we're crafting digital infrastructure.

```{epigraph}
"In the neon glow of the terminal, where keystrokes echo like synthwave beats, we write the code that powers tomorrow."

— nutsloop collective
```
