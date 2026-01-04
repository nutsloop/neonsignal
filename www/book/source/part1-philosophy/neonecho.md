# NeonEcho: The Custom Scripting Language

```{admonition} Future Vision
:class: tip

NeonEcho is an **aspirational design** — a vision for where NeonSignal could go. The language specification below documents the intended syntax and semantics, but **no implementation currently exists**. This is a long-term roadmap item that would require significant development effort (LLVM integration, parser, type system, etc.).
```

NeonEcho is a custom-designed language with LLVM JIT compilation capabilities, created to provide a full application framework within NeonSignal. It is designed for memory efficiency and features rich C++ bindings.

## Language Design Philosophy

Given NeonSignal's HTTP/2 + async I/O architecture, the language is designed to be:

1.  **Async-first:** All I/O is non-blocking by default.
2.  **Strongly typed:** Catches errors at compile time and enables LLVM optimizations.
3.  **Zero-cost abstractions:** High-level syntax with low-level performance.
4.  **Memory safe:** No manual memory management, but predictable allocation.

## Radical Syntax Showcase

NeonEcho is designed to be visually expressive and fun to write. Here are some examples of its unique syntax.

### Basic Wave Handlers

**Simple GET Request:**

```text
@route("GET", "/api/hello")
wave greet_the_world(req: Request) ~> Response {
    wire name = req.query.get("name") <?> "Stranger";

    bounce Response.json({
        message: neon"⚡ Hello, {name}! Welcome to the Grid ⚡",
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
@cache(300)  /// Cache for 5 minutes in the upside down
turbo wave fetch_user(req: Request) ~> Response {
    wire user_id = req.params.id.parse::<u64>()?;

    /// Ride the async wave through the database
    wire user = ride db.query_one::<User>(
        "SELECT id, name, email, bike FROM users WHERE id = $1",
        [user_id]
    )?;

    arcade user {
        jackpot(u) => {
            /// Cache hit - store in the grid
            ride cache.set(neon"user:{user_id}", u, ttl: 300);
            Response.json(u).with_header("X-Cache", "MISS")
        }
        void => Response.error("User not found in the Grid", 404)
    }
}
```

### Entities and Variants

**Entity Declaration (Struct):**

```text
entity Rider {
    broadcast id: u64,
    broadcast name: Text,
    broadcast bike: Text,
    broadcast power_level: u32,
    whisper secret_ability: Text,
    broadcast created_at: Timestamp
}
```

**Variant Declaration (Enum):**

```text
variant GameResult {
    Victory(u64),                    /// score
    Defeat(Text),                    /// reason
    TimeOut,
    Continue {
        level: u32,
        lives: u8,
        boss_health: f32
    }
}
```

## Grammar Specification

The following is a detailed EBNF-style grammar for NeonEcho, presented in its "Radical-BNF" dialect.

- `~>` - Definition
- `|` - Alternative
- `{}` - Zero or more
- `[]` - Zero or one
- `<>` - One or more

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
    | "plug" | "beam" | "cassette"

    /* Literals */
    | "rad" | "bogus" | "void" | "jackpot"
```

### Expressions and Statements

```text
statement ~>
    | expression_pulse
    | wire_statement

expression_pulse ~> expression ";"

wire_statement ~>
    "wire"
    ["flux"]
    pattern
    [type_mark]
    ["=" expression]
    ";"

block_portal ~> "{" <statement>* [expression] "}"

portal_branch ~>
    "portal" expression block_portal
    ["otherwise" (portal_branch | block_portal)]

arcade_match ~>
    "arcade" expression "{"
        [arcade_round <"," arcade_round>* [","]]
    "}"
```

This provides a glimpse into the structure and style of NeonEcho. For the complete grammar, please refer to the source `NeonEcho_Grammar.md` file.
