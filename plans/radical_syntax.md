# 🌴⚡ NEONSCRIPT RADICAL SYNTAX SHOWCASE ⚡🌴

```
    ███╗   ██╗███████╗ ██████╗ ███╗   ██╗
    ████╗  ██║██╔════╝██╔═══██╗████╗  ██║
    ██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║
    ██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║
    ██║ ╚████║███████╗╚██████╔╝██║ ╚████║
    ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝

    WHERE CODE MEETS NEON DREAMS
```

---

## 🎮 BASIC WAVE HANDLERS

### Simple GET Request

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

### With Route Parameters

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

---

## ⚡ TURBO MODE (ASYNC)

### Async Database Query

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

### Parallel Async Operations

```text
turbo wave dashboard(req: Request) ~> Response {
    wire user_id = req.session.user_id;

    /// Launch all queries simultaneously - ride multiple waves!
    wire (user, posts, stats) = ride parallel({
        db.get_user(user_id),
        db.get_user_posts(user_id, limit: 10),
        db.get_user_stats(user_id)
    });

    bounce Response.json({
        user,
        recent_posts: posts,
        statistics: stats,
        grid_status: "ONLINE"
    })
}
```

---

## 🎯 ENTITIES AND VARIANTS

### Entity Declaration (Struct)

```text
entity Rider {
    broadcast id: u64,
    broadcast name: Text,
    broadcast bike: Text,
    broadcast power_level: u32,
    whisper secret_ability: Text,
    broadcast created_at: Timestamp
}

entity Bike {
    broadcast model: Text,
    broadcast max_speed: f32,
    broadcast color: Text,
    broadcast turbos_installed: u8
}
```

### Variant Declaration (Enum)

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

variant ApiResponse {
    Success { data: Text, timestamp: u64 },
    Error { message: Text, code: u32 },
    Redirect(Text),
    NotFound
}
```

---

## 💪 POWERS (IMPLEMENTATIONS)

### Entity Methods

```text
powers Rider {
    /// Constructor - spawn a new rider
    wave spawn(name: Text, bike: Text) ~> Rider {
        Rider {
            id: 0,
            name,
            bike,
            power_level: 1,
            secret_ability: "Ultra Vision",
            created_at: now()
        }
    }

    /// Instance method - check if rad
    wave is_rad(&self) ~> truth {
        self.name.len() > 0
            && self.bike != "rusty"
            && self.power_level > 5
    }

    /// Mutable method - level up the rider
    turbo wave level_up(&flux self) {
        self.power_level += 1;

        ride db.execute(
            "UPDATE riders SET power_level = $1 WHERE id = $2",
            [self.power_level, self.id]
        );

        portal self.power_level > 9000 {
            self.secret_ability = "LEGENDARY MODE UNLOCKED";
        }
    }

    /// Associated function - load from database
    turbo wave load_from_grid(id: u64) ~> Rider? {
        ride db.query_one::<Rider>(
            "SELECT * FROM riders WHERE id = $1",
            [id]
        )
    }
}
```

---

## 🎮 ARCADE MATCHING (PATTERN MATCHING)

### Basic Pattern Matching

```text
wave handle_result(result: GameResult) ~> Response {
    arcade result {
        Victory(score) portal score > 1000 => {
            Response.json({
                message: "🏆 HIGH SCORE! LEGENDARY! 🏆",
                score,
                achievement: "GRID_MASTER",
                reward: 1000
            })
        }

        Victory(score) => {
            Response.json({
                message: "You won!",
                score
            })
        }

        Defeat(reason) => {
            Response.error(neon"💀 Game Over: {reason}", 418)
        }

        TimeOut => {
            Response.error("Time ran out in the Grid", 408)
        }

        Continue { level, lives, boss_health } => {
            Response.json({
                status: "ONGOING",
                level,
                lives,
                boss_health,
                message: "Keep riding! The Grid needs you!"
            })
        }
    }
}
```

### Destructuring Patterns

```text
wave process_user(user: User) {
    /// Destructure entity fields
    wire User { name, bike, power_level, .. } = user;

    arcade (power_level, bike) {
        (p, b) portal p > 9000 && b == "TurboMaxx" => {
            print(neon"⚡ {name} is LEGENDARY with {b}! ⚡");
        }

        (p, _) portal p > 100 => {
            print(neon"💪 {name} is getting strong!");
        }

        _ => {
            print(neon"🌱 {name} is just starting out");
        }
    }
}
```

---

## 🔮 QUANTUM FLIPS (CLOSURES)

### Map/Filter/Reduce Chains

```text
wave process_scores(scores: [i32]) ~> [i32] {
    scores
        |> filter(|x| x > 0)
        |> map(|x| x * 2)
        |> filter(|x| x < 100)
        |> sort_desc()
        |> take(10)
        |> collect()
}
```

### Async Closures

```text
turbo wave batch_update(ids: [u64]) {
    wire results = ids
        |> map(turbo |id| {
            ride db.execute(
                "UPDATE riders SET last_seen = NOW() WHERE id = $1",
                [id]
            )
        })
        |> parallel_all();  /// Execute all async operations concurrently

    print(neon"✅ Updated {results.len()} riders");
}
```

---

## 🌊 CONTROL FLOW

### Portal Branches (If/Else)

```text
wave check_access(user: User, resource: Text) ~> truth {
    portal user.power_level > 100 {
        bounce rad;  /// Admin access
    } otherwise portal user.permissions.contains(resource) {
        bounce rad;  /// Has permission
    } otherwise {
        bounce bogus;  /// Access denied
    }
}
```

### Loops

```text
/// Loop forever (until bail)
wave infinite_runner() {
    wire flux count = 0;

    loop {
        count += 1;

        portal count > 1000 {
            bail;  /// Break out
        }

        portal count % 100 == 0 {
            print(neon"Checkpoint: {count}");
        }
    }
}

/// While loop
wave countdown(flux seconds: u32) {
    while seconds > 0 {
        print(neon"⏰ {seconds} seconds remaining...");
        sleep(1000);
        seconds -= 1;
    }
    print("🎉 BLAST OFF!");
}

/// Chase loop (for-each)
wave greet_all(riders: [Rider]) {
    chase rider within riders {
        print(neon"👋 Hey {rider.name}!");
    }
}
```

---

## 📡 PLUG SYSTEM (IMPORTS/EXPORTS)

### Importing Modules

```text
/// Import entire module
plug "neonsignal:runtime";
plug grid::database;
plug grid::cache morph CacheGrid;

/// Import specific items
plug { User, Rider, Bike } cassette "~/models";
plug { authenticate, authorize morph auth } cassette grid::security;

/// Import from npm-style paths
plug "std:collections" morph collections;
```

### Exporting

```text
/// Export specific items
beam { User, fetch_user, create_user };

/// In another file
plug { User, fetch_user } cassette "~/api/users";
```

---

## ✨ SPECIAL FEATURES

### Neon Format Strings

```text
wire name = "TurboKid";
wire score = 9001;
wire level = 42;

wire message = neon"
    ⚡━━━━━━━━━━━━━━━━━━━━━━━━⚡
    🏆 Player: {name}
    💯 Score:  {score}
    🎮 Level:  {level}
    ⚡━━━━━━━━━━━━━━━━━━━━━━━━⚡
";
```

### ANSI Color Codes in Strings

```text
wire colored = "\e[35m💜 PURPLE NEON \e[0m\e[36m💙 CYAN GLOW \e[0m";
print(colored);
```

### Emoji Identifiers

```text
wire 🌴 = "palm_trees";
wire ⚡ = "electric";
wire 🎮 = "arcade_mode";
wire 🏆 = "victory";

portal 🎮 && ⚡ {
    print(neon"{🌴} {🏆}");
}
```

### Spread Operators

```text
wire base_rider = Rider {
    id: 1,
    name: "Max",
    bike: "Thunder",
    power_level: 50,
    secret_ability: "Speed",
    created_at: now()
};

/// Create modified copy
wire upgraded = Rider {
    ...base_rider,
    power_level: 9001,
    secret_ability: "ULTRA SPEED"
};
```

---

## 🎯 POWER RUNES (ATTRIBUTES)

### HTTP Routing

```text
@route("POST", "/api/riders")
@auth_required
@rate_limit(100)  /// 100 requests per minute
turbo wave create_rider(req: Request) ~> Response {
    wire body = req.json::<CreateRiderRequest>()?;

    wire rider = ride db.execute(
        "INSERT INTO riders (name, bike) VALUES ($1, $2) RETURNING *",
        [body.name, body.bike]
    )?;

    bounce Response.json(rider).with_status(201);
}
```

### Caching and Optimization

```text
@route("GET", "/api/leaderboard")
@cache(ttl: 60)  /// Cache for 60 seconds
@turbo_mode      /// Extra LLVM optimizations
turbo wave leaderboard(req: Request) ~> Response {
    wire top_riders = ride db.query::<Rider>(
        "SELECT * FROM riders ORDER BY power_level DESC LIMIT 100"
    )?;

    bounce Response.json({
        riders: top_riders,
        updated_at: now(),
        grid_status: "⚡ ONLINE ⚡"
    });
}
```

### Custom Runes

```text
@neon("🌴 PALM TREE MODE ACTIVATED 🌴")
@validate(schema: "CreateUserSchema")
@log_performance
turbo wave special_handler(req: Request) ~> Response {
    /// Implementation
}
```

---

## 🔥 ADVANCED PATTERNS

### Error Handling with `?`

```text
turbo wave risky_operation(id: u64) ~> Result<User, Text> {
    /// The ? operator propagates errors up
    wire user = ride db.get_user(id)?;
    wire posts = ride db.get_posts(user.id)?;
    wire stats = ride calculate_stats(posts)?;

    user.stats = stats;

    bounce jackpot(user);
}
```

### Guard Clauses in Arcade

```text
wave classify_rider(rider: Rider) ~> Text {
    arcade rider.power_level {
        p portal p > 9000 => "LEGENDARY 🏆",
        p portal p > 1000 => "ELITE ⚡",
        p portal p > 100 => "VETERAN 💪",
        p portal p > 10 => "RISING STAR 🌟",
        _ => "NEWBIE 🌱"
    }
}
```

### Type Morphing

```text
wave process_data(data: Text) {
    wire number = data.parse::<u64>() <?> 0;
    wire float_val = number morph f64;
    wire text_val = float_val.to_text();

    print(neon"Morphed: {text_val}");
}
```

---

## 🌴 PROTOCOLS (TRAITS)

### Define a Protocol

```text
protocol Rideable {
    wave max_speed(&self) ~> f32;
    wave accelerate(&flux self, amount: f32);
    wave brake(&flux self);
}

protocol GridEntity {
    wave to_json(&self) ~> Text;
    wave from_grid(id: u64) ~> Self?;
}
```

### Implement Protocol

```text
powers Rideable for Bike {
    wave max_speed(&self) ~> f32 {
        self.base_speed * (1.0 + 0.2 * self.turbos_installed morph f32)
    }

    wave accelerate(&flux self, amount: f32) {
        self.current_speed += amount;
        portal self.current_speed > self.max_speed() {
            self.current_speed = self.max_speed();
        }
    }

    wave brake(&flux self) {
        self.current_speed *= 0.8;
    }
}
```

---

## 🎮 COMPLETE EXAMPLE: GAME API

```text
plug { db, cache } cassette "neonsignal:runtime";
plug { User, GameSession, Score } cassette "~/models";

/// Create new game session
@route("POST", "/api/game/start")
@auth_required
turbo wave start_game(req: Request) ~> Response {
    wire user_id = req.session.user_id;
    wire user = ride db.get_user(user_id)?;

    /// Check if already in game
    wire existing = ride cache.get(neon"session:{user_id}");
    arcade existing {
        jackpot(_) => bounce Response.error("Already in game!", 409),
        void => {}  /// Continue
    }

    /// Create new session
    wire session = GameSession {
        id: generate_id(),
        user_id,
        level: 1,
        lives: 3,
        score: 0,
        started_at: now()
    };

    /// Store in cache
    ride cache.set(neon"session:{user_id}", session, ttl: 3600);

    bounce Response.json({
        session_id: session.id,
        message: neon"🎮 Game started! Good luck, {user.name}! 🎮"
    });
}

/// Submit score
@route("POST", "/api/game/score")
@auth_required
@rate_limit(10)
turbo wave submit_score(req: Request) ~> Response {
    wire user_id = req.session.user_id;
    wire body = req.json::<SubmitScoreRequest>()?;

    /// Validate session
    wire session = ride cache.get(neon"session:{user_id}")
        <?> bounce Response.error("No active session", 404);

    /// Save score
    wire score = Score {
        user_id,
        value: body.score,
        level: session.level,
        timestamp: now()
    };

    ride db.save_score(score);

    /// Check for high score
    wire is_high_score = ride db.is_high_score(user_id, body.score);

    wire message = arcade is_high_score {
        rad => neon"🏆 NEW HIGH SCORE: {body.score}! LEGENDARY! 🏆",
        bogus => neon"Score submitted: {body.score}"
    };

    bounce Response.json({ message });
}

/// Leaderboard
@route("GET", "/api/game/leaderboard")
@cache(300)
turbo wave leaderboard(req: Request) ~> Response {
    wire limit = req.query.get("limit")
        .and_then(|s| s.parse::<u32>())
        <?> 10;

    wire top_scores = ride db.query::<Score>(
        "SELECT * FROM scores ORDER BY value DESC LIMIT $1",
        [limit]
    )?;

    bounce Response.json({
        scores: top_scores,
        grid_status: "⚡ ONLINE ⚡",
        last_updated: now()
    });
}
```

---

## 🌊 CLOSING TRANSMISSION

This is **NeonEcho** - where every line pulses with electric energy.

**No boring syntax.**
**No conventional rules.**
**Just pure, neon-soaked CODE.**

Welcome to the Grid. 🌴⚡🎮

```
    ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    █ END TRANSMISSION  [████] █
    ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
```
