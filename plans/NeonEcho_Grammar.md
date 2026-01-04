# ⚡ NEONSCRIPT GRAMMAR SPECIFICATION ⚡
## A Synthwave-Powered Language for the Electric Future

```
    ███╗   ██╗███████╗ ██████╗ ███╗   ██╗
    ████╗  ██║██╔════╝██╔═══██╗████╗  ██║
    ██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║
    ██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║
    ██║ ╚████║███████╗╚██████╔╝██║ ╚████║
    ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝
    S C R I P T   //  R I D E   T H E   W A V E
```

---

## 📼 Notation (Because Rules Are Made to Be Broken)

EBNF? More like **RADICAL-BNF**:

- `~>` - Definition (because arrows are cooler than `::=`)
- `|` - Alternative (this one's already rad)
- `{}` - Loop it (zero or more, like a sick synth loop)
- `[]` - Maybe (zero or one, Schrödinger style)
- `<>` - Required (one or more, crank it to 11)
- `""` - Literal strings (neon signs)
- `/* */` - Comments from the upside down

---

## 🎮 1. THE LEXICON OF POWER

### 1.1 Sacred Keywords (The Electric Vocabulary)

```text
keyword ~>
    /* Flow Control - Ride the Lightning */
    | "wave"          /* function declaration - send a signal */
    | "turbo"         /* async modifier - maximum speed */
    | "ride"          /* await - ride the async wave */
    | "wire"          /* let binding - jack into the grid */
    | "static"        /* const - unchanging like the void */
    | "flux"          /* mut modifier - things change in the upside down */

    /* Control Structures - Navigate the Grid */
    | "portal"        /* if - gateway to another dimension */
    | "otherwise"     /* else - the other side */
    | "arcade"        /* match - choose your fighter */
    | "bounce"        /* return - echo back */
    | "bail"          /* break - escape the loop */
    | "cruise"        /* continue - keep riding */

    /* Loops - Infinite Runners */
    | "loop"          /* infinite loop - forever young */
    | "while"         /* conditional loop - classic mode */
    | "chase"         /* for loop - pursuit mode */

    /* Type Definitions - Building Blocks of Reality */
    | "entity"        /* struct - a thing in the grid */
    | "variant"       /* enum - multiple realities */
    | "powers"        /* impl - supernatural abilities */
    | "protocol"      /* trait - the rules of the game */

    /* Module System - Plug Into The Matrix */
    | "plug"          /* import - load the cassette */
    | "beam"          /* export - transmit the signal */
    | "cassette"      /* from - source of power */

    /* Visibility - Who Sees the Truth */
    | "broadcast"     /* pub - everyone can hear */
    | "whisper"       /* priv - secret transmission */

    /* Literals - The Constants */
    | "rad"           /* true - totally rad */
    | "bogus"         /* false - that's bogus */
    | "void"          /* None - the empty void */
    | "jackpot"       /* Some - hit the jackpot */

    /* Operators - The Glue */
    | "morph"         /* as - shapeshift type */
    | "within"        /* in - inside the boundary */
```

### 1.2 Identifiers (Your Handle in the Grid)

```text
identifier ~> (letter | "_" | "$") <letter | digit | "_" | "$">*

letter ~> "a".."z" | "A".."Z" | "🌴" | "⚡" | "🎮" | "📼" | "🌊"  /* Emoji allowed! */
digit ~> "0".."9"
```

### 1.3 Literals (The Building Blocks of Reality)

```text
literal ~>
    | number_literal
    | text_literal
    | glyph_literal
    | truth_literal
    | sequence_literal
    | grid_literal

/* Numbers - The Mathematics of the Universe */
number_literal ~>
    | decimal_glow      /* normal numbers */
    | hex_glow          /* hexadecimal - for the hackers */
    | binary_pulse      /* binary - machine language */
    | float_wave        /* floating point - ride the wave */

decimal_glow ~> digit <digit | "_">*

hex_glow ~> "0x" hex_digit <hex_digit | "_">*

binary_pulse ~> "0b" ("0" | "1") <"0" | "1" | "_">*

hex_digit ~> digit | "a".."f" | "A".."F"

float_wave ~> digit <digit | "_">* "." digit <digit | "_">* [exponent]
            | digit <digit | "_">* exponent

exponent ~> ("e" | "E") ["+" | "-"] digit <digit>*

/* Text - Messages from Beyond */
text_literal ~>
    | '"' <text_char | escape_portal>* '"'
    | "neon" '"' <text_char | escape_portal | warp_field>* '"'  /* Neon-lit format strings! */

escape_portal ~>
    | "\\" ("n" | "t" | "r" | "\\" | '"' | "'" | "0")
    | "\\x" hex_digit hex_digit
    | "\\u{" hex_digit <hex_digit>* "}"
    | "\\e" "[" <digit | ";">* "m"  /* ANSI escape codes - because why not? */

warp_field ~> "{" expression "}"  /* Interpolation - warp reality */

/* Single Character - The Atom */
glyph_literal ~> "'" (char | escape_portal) "'"

/* Boolean - The Binary Choice */
truth_literal ~> "rad" | "bogus"

/* Arrays - Sequences in Time */
sequence_literal ~> "[" [expression {"," expression} [","]] "]"

/* Objects - Nodes in the Grid */
grid_literal ~> "{" [grid_node {"," grid_node} [","]] "}"

grid_node ~>
    | identifier ":" expression
    | identifier                /* shorthand - because we're lazy */
    | "..." expression          /* spread operator - scatter the data */
```

### 1.4 Operators (The Connectors)

```text
operator ~>
    /* Arithmetic - The Math Wizards */
    | "+" | "-" | "*" | "/" | "%" | "**"

    /* Comparison - Judge and Jury */
    | "==" | "!=" | "<" | ">" | "<=" | ">="

    /* Logical - The Philosophers */
    | "&&" | "||" | "!"

    /* Bitwise - The Hackers */
    | "&" | "|" | "^" | "<<" | ">>"

    /* Assignment - The Mutators */
    | "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "~="

    /* Special - The Weird Ones */
    | "." | ".." | "..=" | "?"
    | "~>" | "=>" | "::" | "@"
    | "|>" /* pipe operator - unix philosophy */
    | "<?>" /* quantum maybe operator */
```

### 1.5 Punctuation (The Delimiters)

```text
punctuation ~> "(" | ")" | "{" | "}" | "[" | "]" | "," | ";" | ":"
```

### 1.6 Comments (Transmissions from the Void)

```text
comment ~> line_transmission | block_transmission

line_transmission ~> "//" <any_char>* newline

block_transmission ~> "/*" <any_char | block_transmission>* "*/"  /* Nested! */

/* Special: Neon Comments (rendered in glowing text) */
neon_comment ~> "///" <any_char>* newline  /* Triple slash for documentation */
```

---

## 🌊 2. PROGRAM STRUCTURE (The Architecture of Dreams)

### 2.1 Source Cassette

```text
cassette_file ~> <power_rune>* <module_item>*

module_item ~>
    | plug_declaration      /* Load external powers */
    | wave_declaration      /* Signal definitions */
    | entity_declaration    /* Data structures */
    | variant_declaration   /* Multiple realities */
    | powers_declaration    /* Supernatural abilities */
    | protocol_declaration  /* The rules */
    | static_declaration    /* Immutable truths */
    | type_warp             /* Type aliases */
```

### 2.2 Module System (Plug Into The Grid)

```text
plug_declaration ~> "plug" plug_spec

plug_spec ~>
    | plug_path ["morph" identifier]
    | "{" plug_item <"," plug_item>* [","] "}" "cassette" plug_path

plug_path ~>
    | text_literal                           /* "neonsignal:runtime" */
    | identifier <"::" identifier>*          /* grid::database */
    | "~/" identifier <"/" identifier>*      /* local paths */

plug_item ~> identifier ["morph" identifier]

/* Broadcasting to the world */
beam_declaration ~> "beam" (identifier | "{" identifier <"," identifier>* "}")
```

---

## ⚡ 3. TYPE SYSTEM (The Fabric of Reality)

### 3.1 Type Syntax

```text
type_form ~>
    | primitive_energy
    | identifier [type_boost]     /* Custom types */
    | sequence_type
    | cluster_type
    | wave_type
    | maybe_type
    | result_type
    | reference_type

/* Primitive Energies - The Base Elements */
primitive_energy ~>
    /* Unsigned Integers - Pure Energy */
    | "u8" | "u16" | "u32" | "u64" | "u128"

    /* Signed Integers - Positive/Negative Charge */
    | "i8" | "i16" | "i32" | "i64" | "i128"

    /* Floating Point - Wave Functions */
    | "f32" | "f64"

    /* Other Primitives */
    | "truth"         /* boolean */
    | "glyph"         /* char */
    | "Text"          /* String */
    | "signal"        /* str (string slice) */
    | "void"          /* unit type */

/* Collections */
sequence_type ~>
    | "[" type_form "]"                    /* dynamic array */
    | "[" type_form ";" expression "]"     /* fixed size */

cluster_type ~> "(" [type_form <"," type_form>* [","]] ")"

wave_type ~> "wave" "(" [type_form <"," type_form>*] ")" ["~>" type_form]

/* Maybe/Error - Schrödinger's Types */
maybe_type ~>
    | "Maybe" "<" type_form ">"
    | type_form "?"                        /* shorthand - quantum uncertainty */

result_type ~> "Result" "<" type_form "," type_form ">"

/* References - Pointers to Reality */
reference_type ~> "&" ["flux"] type_form

/* Type Arguments - Power Boosters */
type_boost ~> "<" type_form <"," type_form>* [","] ">"
```

### 3.2 Type Annotations

```text
type_mark ~> ":" type_form
```

---

## 🎮 4. DECLARATIONS (Building the World)

### 4.1 Wave Declarations (Functions)

```text
wave_declaration ~>
    <power_rune>*
    ["broadcast"]
    ["turbo"]
    "wave"
    identifier
    [type_params]
    "(" [param_list] ")"
    ["~>" type_form]
    block_portal

param_list ~> parameter <"," parameter>* [","]

parameter ~>
    identifier type_mark ["=" expression]    /* Default values! */

type_params ~> "<" type_param <"," type_param>* [","] ">"

type_param ~> identifier [":" protocol_bounds]

protocol_bounds ~> identifier <"+" identifier>*
```

### 4.2 Entity Declarations (Structs)

```text
entity_declaration ~>
    <power_rune>*
    ["broadcast"]
    "entity"
    identifier
    [type_params]
    entity_body

entity_body ~>
    | "{" [entity_slot <"," entity_slot>* [","]] "}"    /* Record entity */
    | "(" [type_form <"," type_form>* [","]] ")" ";"    /* Tuple entity */
    | ";"                                                /* Unit entity */

entity_slot ~>
    <power_rune>*
    ["broadcast"]
    identifier
    type_mark
```

### 4.3 Variant Declarations (Enums)

```text
variant_declaration ~>
    <power_rune>*
    ["broadcast"]
    "variant"
    identifier
    [type_params]
    "{"
        [variant_choice <"," variant_choice>* [","]]
    "}"

variant_choice ~>
    <power_rune>*
    identifier
    [variant_payload]

variant_payload ~>
    | "(" [type_form <"," type_form>* [","]] ")"              /* Tuple */
    | "{" [entity_slot <"," entity_slot>* [","]] "}"          /* Struct */
```

### 4.4 Powers Declarations (Impl Blocks)

```text
powers_declaration ~>
    "powers"
    [type_params]
    [protocol_path "for"]
    type_form
    "{"
        <powers_item>*
    "}"

powers_item ~>
    | wave_declaration
    | static_declaration

protocol_path ~> identifier <"::" identifier>*
```

### 4.5 Protocol Declarations (Traits)

```text
protocol_declaration ~>
    <power_rune>*
    ["broadcast"]
    "protocol"
    identifier
    [type_params]
    "{"
        <protocol_item>*
    "}"

protocol_item ~>
    | wave_signature
    | static_declaration

wave_signature ~>
    "wave"
    identifier
    [type_params]
    "(" [param_list] ")"
    ["~>" type_form]
    ";"
```

### 4.6 Static Declarations (Constants)

```text
static_declaration ~>
    "static"
    identifier
    type_mark
    "="
    expression
    ";"
```

### 4.7 Type Warps (Aliases)

```text
type_warp ~>
    "type"
    identifier
    [type_params]
    "="
    type_form
    ";"
```

---

## 🌴 5. STATEMENTS (Actions in Time)

```text
statement ~>
    | expression_pulse
    | wire_statement
    | module_item

expression_pulse ~> expression ";"

wire_statement ~>
    "wire"
    ["flux"]
    pattern
    [type_mark]
    ["=" expression]
    ";"
```

---

## 🔮 6. EXPRESSIONS (The Language of Power)

### 6.1 Expression Hierarchy (Precedence Ladder)

```text
expression ~> assignment_blast

/* Level 15: Assignment - The Great Mutation */
assignment_blast ~>
    logic_or_flow [assignment_op assignment_blast]

assignment_op ~> "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "~="

/* Level 14: Logical OR - Choose Your Reality */
logic_or_flow ~>
    logic_and_flow <"||" logic_and_flow>*

/* Level 13: Logical AND - Both Must Be True */
logic_and_flow ~>
    equality_check <"&&" equality_check>*

/* Level 12: Equality - The Truth */
equality_check ~>
    relation_check <("==" | "!=") relation_check>*

/* Level 11: Relational - Size Matters */
relation_check ~>
    bit_or_merge <("<" | ">" | "<=" | ">=") bit_or_merge>*

/* Level 10: Bitwise OR */
bit_or_merge ~>
    bit_xor_flip <"|" bit_xor_flip>*

/* Level 9: Bitwise XOR */
bit_xor_flip ~>
    bit_and_mask <"^" bit_and_mask>*

/* Level 8: Bitwise AND */
bit_and_mask ~>
    bit_shift_slide <"&" bit_shift_slide>*

/* Level 7: Bit Shift */
bit_shift_slide ~>
    addition_sum <("<<" | ">>") addition_sum>*

/* Level 6: Addition/Subtraction */
addition_sum ~>
    multiply_product <("+" | "-") multiply_product>*

/* Level 5: Multiplication/Division */
multiply_product ~>
    power_surge <("*" | "/" | "%") power_surge>*

/* Level 4: Exponentiation */
power_surge ~>
    unary_transform <"**" unary_transform>*

/* Level 3: Unary Operations */
unary_transform ~>
    | postfix_chain
    | unary_op unary_transform

unary_op ~> "-" | "!" | "&" | "*"

/* Level 2: Postfix Operations */
postfix_chain ~>
    primary_atom <postfix_op>*

postfix_op ~>
    | call_blast
    | index_dive
    | field_reach
    | method_invoke
    | quantum_try
    | morph_cast
    | pipe_flow

call_blast ~> "(" [arg_list] ")"

arg_list ~> expression <"," expression>* [","]

index_dive ~> "[" expression "]"

field_reach ~> "." identifier

method_invoke ~> "." identifier [type_boost] "(" [arg_list] ")"

quantum_try ~> "?"

morph_cast ~> "morph" type_form

pipe_flow ~> "|>" expression  /* Unix-style pipe */

/* Level 1: Primary - The Atoms */
primary_atom ~>
    | literal
    | identifier
    | "(" expression ")"
    | block_portal
    | portal_branch
    | arcade_match
    | loop_forever
    | loop_while
    | chase_loop
    | bounce_back
    | bail_out
    | cruise_on
    | ride_wave
    | quantum_flip
    | entity_spawn
    | sequence_build
    | cluster_build
```

### 6.2 Control Flow (Navigate the Grid)

```text
block_portal ~> "{" <statement>* [expression] "}"

portal_branch ~>
    "portal" expression block_portal
    ["otherwise" (portal_branch | block_portal)]

arcade_match ~>
    "arcade" expression "{"
        [arcade_round <"," arcade_round>* [","]]
    "}"

arcade_round ~>
    pattern
    ["portal" expression]  /* guard */
    "=>"
    (expression | block_portal)

loop_forever ~> "loop" block_portal

loop_while ~> "while" expression block_portal

chase_loop ~> "chase" pattern "within" expression block_portal

bounce_back ~> "bounce" [expression]

bail_out ~> "bail" [expression]

cruise_on ~> "cruise"
```

### 6.3 Async/Turbo Mode

```text
ride_wave ~> "ride" expression

/* Turbo wave - async function */
turbo_wave ~>
    ["turbo"]
    "|" [closure_params] "|"
    ["~>" type_form]
    (expression | block_portal)
```

### 6.4 Quantum Operations (Closures)

```text
quantum_flip ~>
    ["turbo"]
    "|" [closure_params] "|"
    ["~>" type_form]
    (expression | block_portal)

closure_params ~> closure_param <"," closure_param>* [","]

closure_param ~> pattern [type_mark]
```

### 6.5 Entity Spawning (Struct Instantiation)

```text
entity_spawn ~>
    identifier "{"
        [entity_slot_init <"," entity_slot_init>* [","]]
    "}"

entity_slot_init ~>
    | identifier ":" expression
    | identifier                    /* shorthand */
    | "..." expression              /* spread/update syntax */
```

### 6.6 Sequences and Clusters

```text
sequence_build ~>
    | "[" [expression <"," expression>* [","]] "]"
    | "[" expression ";" expression "]"  /* repeat */

cluster_build ~> "(" [expression <"," expression>* [","]] ")"
```

---

## 🎯 7. PATTERNS (The Art of Matching)

```text
pattern ~>
    | wildcard_void
    | identifier_bind
    | literal
    | cluster_pattern
    | entity_pattern
    | variant_pattern
    | or_pattern
    | reference_pattern

wildcard_void ~> "_"

identifier_bind ~>
    ["flux"]
    identifier
    ["@" pattern]  /* bind and match deeper */

cluster_pattern ~>
    "(" [pattern <"," pattern>* [","]] ")"

entity_pattern ~>
    identifier "{"
        [field_pattern <"," field_pattern>* [","]]
        [".."]  /* ignore rest */
    "}"

field_pattern ~>
    | identifier [":" pattern]
    | ".."

variant_pattern ~>
    identifier
    ["::" identifier]
    [variant_pattern_data]

variant_pattern_data ~>
    | "(" [pattern <"," pattern>* [","]] ")"
    | "{" [field_pattern <"," field_pattern>* [","]] "}"

or_pattern ~> pattern <"|" pattern>+

reference_pattern ~> "&" ["flux"] pattern
```

---

## ⚡ 8. POWER RUNES (Attributes)

```text
power_rune ~> "@" identifier ["(" rune_args ")"]

rune_args ~>
    | literal
    | identifier "=" literal
    | rune_args "," rune_args

/* Special Runes */
route_rune ~> "@route" "(" text_literal "," text_literal ")"  /* HTTP routing */
cache_rune ~> "@cache" "(" number_literal ")"                 /* Cache TTL */
turbo_rune ~> "@turbo_mode"                                   /* Extra optimization */
neon_rune ~> "@neon" "(" text_literal ")"                     /* Emit neon glow in logs */
```

---

## 🌊 9. OPERATOR PRECEDENCE (Power Levels)

**From MAXIMUM POWER to Minimum:**

1. **Field/Method/Index** (`.`, `[]`, `()`) - Direct connection
2. **Unary** (`-`, `!`, `*`, `&`) - Self modification
3. **Power** (`**`) - Exponential growth
4. **Multiply/Divide** (`*`, `/`, `%`) - Distribution
5. **Add/Subtract** (`+`, `-`) - Combination
6. **Bit Shift** (`<<`, `>>`) - Slide the bits
7. **Bitwise AND** (`&`) - Mask reality
8. **Bitwise XOR** (`^`) - Flip the script
9. **Bitwise OR** (`|`) - Merge realities
10. **Comparison** (`<`, `>`, `<=`, `>=`) - Judge the value
11. **Equality** (`==`, `!=`) - Find the truth
12. **Logical AND** (`&&`) - Both conditions
13. **Logical OR** (`||`) - Either condition
14. **Pipe** (`|>`) - Flow of data
15. **Assignment** (`=`, `+=`, etc.) - Mutate the world

---

## 🎮 10. CODE EXAMPLES (See It In Action)

### Example 1: Simple Wave Handler

```text
@route("GET", "/api/hello")
wave greet_the_world(req: Request) ~> Response {
    wire name = req.query.get("name") <?> "Stranger";
    Response.json({
        message: neon"⚡ Hello, {name}! Welcome to the Grid ⚡",
        timestamp: now(),
        vibes: "rad"
    })
}
```

### Example 2: Turbo-Charged Async

```text
@route("GET", "/api/users/:id")
@cache(300)  // Cache for 5 minutes
turbo wave fetch_user(req: Request) ~> Response {
    wire user_id = req.params.id.parse::<u64>()?;

    // Ride the async wave
    wire user = ride db.query_one::<User>(
        "SELECT * FROM users WHERE id = $1",
        [user_id]
    )?;

    arcade user {
        jackpot(u) => {
            ride cache.set(neon"user:{user_id}", u, ttl: 300);
            Response.json(u).neon_glow("🌴")
        }
        void => Response.error("User not found in the Grid", 404)
    }
}
```

### Example 3: Entity with Powers

```text
entity Rider {
    broadcast id: u64,
    broadcast name: Text,
    broadcast bike: Text,
    whisper secret_power: Text,
}

powers Rider {
    wave spawn(name: Text, bike: Text) ~> Rider {
        Rider {
            id: 0,
            name,
            bike,
            secret_power: "Ultra Vision"
        }
    }

    wave is_rad(&self) ~> truth {
        self.name.len() > 0 && self.bike != "rusty"
    }

    turbo wave level_up(&flux self) {
        ride db.execute(
            "UPDATE riders SET power_level = power_level + 1 WHERE id = $1",
            [self.id]
        );
    }
}
```

### Example 4: Variant and Pattern Arcade

```text
variant GameResult {
    Victory(u64),           // score
    Defeat(Text),           // reason
    Continue {
        level: u32,
        lives: u8
    }
}

wave handle_game_over(result: GameResult) ~> Response {
    arcade result {
        Victory(score) portal score > 1000 =>
            Response.json({
                message: "🏆 HIGH SCORE! 🏆",
                score,
                achievement: "LEGEND"
            }),

        Victory(score) =>
            Response.json({ message: "You won!", score }),

        Defeat(reason) =>
            Response.error(neon"💀 Game Over: {reason}", 418),

        Continue { level, lives } =>
            Response.json({
                status: "ongoing",
                level,
                lives,
                message: "Keep riding!"
            })
    }
}
```

### Example 5: Quantum Flip (Closure)

```text
wave process_data(numbers: [i32]) ~> [i32] {
    numbers
        |> filter(|x| x > 0)
        |> map(|x| x * 2)
        |> filter(|x| x < 100)
        |> collect()
}
```

---

## 🔮 11. RESERVED FOR FUTURE POWER-UPS

These keywords are locked in the vault for future expansions:

- `dimension` - for higher-dimensional types
- `quantum` - for quantum computing primitives
- `glitch` - for debugging/panic modes
- `rewind` - for time-travel debugging
- `cassette_a` / `cassette_b` - for A/B testing
- `multiverse` - for parallel execution
- `arcade_mode` - for game loop specific features
- `synth` - for code generation/macros
- `vhs` - for legacy compatibility mode

---

## 🌴 12. GRAMMAR PHILOSOPHY

### The Synthwave Manifesto

**1. Visual > Verbose**
Code should look like neon art, not legal documents.

**2. Expressiveness > Efficiency**
We have JIT compilation. Make it *fun*.

**3. Emoji Are Valid Identifiers**
Because the future is now. `let 🌴palm_trees = "cool";`

**4. Color Is Syntax**
(When rendered in a proper terminal, keywords glow)

**5. Pipes Over Parentheses**
`data |> transform |> filter |> render` beats `render(filter(transform(data)))`

**6. Break The Rules**
If it makes the code more rad, do it.

---

## 🎯 13. PARSING STRATEGY

### The Recursive Descent Into The Grid

**Lexer Phase - Tokenization:**
```
Source → Tokens (with neon glow metadata)
```

**Parser Phase - AST Construction:**
```
Tokens → AST (decorated with power runes)
```

**Type Check Phase - Reality Validation:**
```
AST → Typed AST (all types resolved)
```

**IR Generation - Lower to LLVM:**
```
Typed AST → LLVM IR (ready for the machine)
```

**JIT Compilation - Materialize:**
```
LLVM IR → Native Code (execute in the Grid)
```

---

## ⚡ IMPLEMENTATION NOTES

### Character Encoding
- **UTF-8 Required** - We live in the future
- **Emoji Support** - First-class identifiers
- **ANSI Escape Codes** - In string literals for colored output
- **Neon Strings** - Prefix with `neon` for auto-colored interpolation

### Error Messages
All errors should be delivered with **maximum drama**:

```
⚠️  GRID MALFUNCTION ⚠️
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📍 Location: riders.neon:42:8
🔴 Error: Type mismatch in the upside down

   Expected: Text
   Found:    u64

💡 Hint: Try using .to_text() to morph the type
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🌊 CLOSING TRANSMISSION

This language doesn't follow the rules. It **rewrites** them.

Every keyword pulses with neon energy.
Every operator flows like synthwave.
Every program is a ride through the grid.

**Welcome to NeonEcho.**

**Now... let's ride. 🌴⚡🎮**

```
    ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    █ END OF TRANSMISSION  [▓▓▓▓] █
    ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
```
