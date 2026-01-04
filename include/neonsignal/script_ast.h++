#pragma once

/**
 * ⚡ NEONECHO AST - THE ELECTRIC SYNTAX TREE ⚡
 *
 *   ███╗   ██╗███████╗ ██████╗ ███╗   ██╗
 *   ████╗  ██║██╔════╝██╔═══██╗████╗  ██║
 *   ██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║
 *   ██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║
 *   ██║ ╚████║███████╗╚██████╔╝██║ ╚████║
 *   ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * AST Nodes for the Language of the Future
 * Where Every Node Pulses with Neon Energy 🌴⚡
 */

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <cstdint>

namespace neonsignal::neonecho::ast {

// Forward declarations - The Grid Awaits
struct Expression;
struct Statement;
struct Pattern;
struct TypeForm;

// ╔══════════════════════════════════════════════════════════════╗
// ║  📍 SOURCE LOCATION TRACKING - WHERE IN THE GRID ARE WE?      ║
// ╚══════════════════════════════════════════════════════════════╝

struct GridCoordinate {
    std::string cassette_name;  // file name - what tape are we on?
    uint32_t line;              // line number - vertical position
    uint32_t column;            // column number - horizontal position
    uint32_t byte_offset;       // absolute byte position in the stream
};

struct GridSpan {
    GridCoordinate entrance;    // where we jack in
    GridCoordinate exit;        // where we disconnect
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  🎯 IDENTIFIERS - YOUR HANDLE IN THE GRID                     ║
// ╚══════════════════════════════════════════════════════════════╝

struct Handle {
    std::string name;           // your identifier in the grid
    GridSpan location;
};

struct DataPath {
    std::vector<Handle> segments;  // path through the grid: grid::database::query
    GridSpan location;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  ⚡ TYPE SYSTEM - THE FABRIC OF REALITY                       ║
// ╚══════════════════════════════════════════════════════════════╝

struct PrimitiveEnergy {
    enum class Voltage {
        // Unsigned - Pure positive energy
        U8, U16, U32, U64, U128,
        // Signed - Both polarities
        I8, I16, I32, I64, I128,
        // Floating - Wave functions
        F32, F64,
        // Other fundamental particles
        Truth,      // bool
        Glyph,      // char
        Text,       // String
        Signal,     // str (string slice)
        Void        // unit type ()
    };
    Voltage charge;
    GridSpan location;
};

struct SequenceType {
    std::unique_ptr<TypeForm> element_energy;
    std::optional<std::unique_ptr<Expression>> fixed_capacity;  // for [T; N]
    GridSpan location;
};

struct ClusterType {
    std::vector<std::unique_ptr<TypeForm>> elements;  // tuple types
    GridSpan location;
};

struct WaveType {
    std::vector<std::unique_ptr<TypeForm>> param_energies;
    std::unique_ptr<TypeForm> bounce_energy;  // return type
    GridSpan location;
};

struct NamedEnergy {
    DataPath path;
    std::vector<std::unique_ptr<TypeForm>> power_boost;  // type arguments
    GridSpan location;
};

struct MaybeEnergy {
    std::unique_ptr<TypeForm> quantum_state;  // Option<T> / T?
    GridSpan location;
};

struct ResultEnergy {
    std::unique_ptr<TypeForm> success_state;
    std::unique_ptr<TypeForm> failure_state;
    GridSpan location;
};

struct ReferenceEnergy {
    std::unique_ptr<TypeForm> target;
    bool is_flux;  // mutable reference
    GridSpan location;
};

struct TypeForm {
    std::variant<
        PrimitiveEnergy,
        SequenceType,
        ClusterType,
        WaveType,
        NamedEnergy,
        MaybeEnergy,
        ResultEnergy,
        ReferenceEnergy
    > essence;

    GridSpan location() const;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  💎 LITERALS - THE BUILDING BLOCKS OF REALITY                 ║
// ╚══════════════════════════════════════════════════════════════╝

struct NumberGlow {
    enum class Radix { Decimal, Hex, Binary, Octal };
    uint64_t value;
    Radix base;
    GridSpan location;
};

struct FloatWave {
    double amplitude;  // the actual value
    GridSpan location;
};

struct TextBeam {
    std::string transmission;
    bool is_neon;  // true for neon"..." format strings
    GridSpan location;
};

struct GlyphAtom {
    char32_t particle;  // single unicode character
    GridSpan location;
};

struct TruthValue {
    bool is_rad;  // true = rad, false = bogus
    GridSpan location;
};

struct SequenceLiteral {
    std::vector<std::unique_ptr<Expression>> elements;
    std::optional<std::unique_ptr<Expression>> repeat_pulse;  // for [x; n]
    GridSpan location;
};

struct GridLiteral {
    struct Node {
        Handle key;
        std::unique_ptr<Expression> value;
        GridSpan location;
    };
    std::vector<Node> connections;
    GridSpan location;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  🎯 PATTERNS - THE ART OF MATCHING                            ║
// ╚══════════════════════════════════════════════════════════════╝

struct WildcardVoid {
    GridSpan location;  // the _ pattern - matches anything
};

struct HandleBind {
    Handle name;
    bool is_flux;  // mutable binding
    std::optional<std::unique_ptr<Pattern>> deeper_pattern;  // for x @ Pattern
    GridSpan location;
};

struct ClusterPattern {
    std::vector<std::unique_ptr<Pattern>> elements;
    GridSpan location;
};

struct EntityPattern {
    DataPath path;
    struct Slot {
        Handle name;
        std::optional<std::unique_ptr<Pattern>> pattern;
        GridSpan location;
    };
    std::vector<Slot> slots;
    bool has_wildcard_rest;  // for { x, y, .. }
    GridSpan location;
};

struct VariantPattern {
    DataPath path;
    std::variant<
        std::monostate,                           // unit variant
        std::vector<std::unique_ptr<Pattern>>,    // tuple variant
        std::vector<EntityPattern::Slot>          // struct variant
    > payload;
    GridSpan location;
};

struct OrPattern {
    std::vector<std::unique_ptr<Pattern>> choices;  // a | b | c
    GridSpan location;
};

struct ReferencePattern {
    std::unique_ptr<Pattern> target;
    bool is_flux;
    GridSpan location;
};

struct Pattern {
    std::variant<
        WildcardVoid,
        HandleBind,
        NumberGlow,
        TextBeam,
        TruthValue,
        ClusterPattern,
        EntityPattern,
        VariantPattern,
        OrPattern,
        ReferencePattern
    > form;

    GridSpan location() const;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  ⚡ EXPRESSIONS - THE LANGUAGE OF POWER                       ║
// ╚══════════════════════════════════════════════════════════════╝

struct BinaryBlast {
    enum class Operator {
        // Arithmetic - The Math Wizards
        Add, Sub, Mul, Div, Mod, Power,
        // Comparison - Judge and Jury
        Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual,
        // Logical - The Philosophers
        LogicAnd, LogicOr,
        // Bitwise - The Hackers
        BitAnd, BitOr, BitXor, ShiftLeft, ShiftRight,
        // Assignment - The Mutators
        Assign, AddAssign, SubAssign, MulAssign, DivAssign, ModAssign, FluxAssign
    };
    Operator operation;
    std::unique_ptr<Expression> left_charge;
    std::unique_ptr<Expression> right_charge;
    GridSpan location;
};

struct UnaryTransform {
    enum class Operator { Negate, LogicNot, Deref, Ref };
    Operator operation;
    std::unique_ptr<Expression> target;
    GridSpan location;
};

struct CallBlast {
    std::unique_ptr<Expression> wave_source;  // the function being called
    std::vector<std::unique_ptr<Expression>> energy_args;
    GridSpan location;
};

struct MethodInvoke {
    std::unique_ptr<Expression> receiver;
    Handle method_name;
    std::vector<std::unique_ptr<TypeForm>> power_boost;  // type args
    std::vector<std::unique_ptr<Expression>> energy_args;
    GridSpan location;
};

struct FieldReach {
    std::unique_ptr<Expression> entity;
    Handle slot_name;
    GridSpan location;
};

struct IndexDive {
    std::unique_ptr<Expression> sequence;
    std::unique_ptr<Expression> coordinate;
    GridSpan location;
};

struct MorphCast {
    std::unique_ptr<Expression> source;
    std::unique_ptr<TypeForm> target_form;
    GridSpan location;
};

struct QuantumTry {
    std::unique_ptr<Expression> risky_operation;  // expr?
    GridSpan location;
};

struct PipeFlow {
    std::unique_ptr<Expression> data_stream;
    std::unique_ptr<Expression> transformer;  // data |> transformer
    GridSpan location;
};

struct PortalBranch {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> reality_a;  // then branch
    std::optional<std::unique_ptr<Expression>> reality_b;  // else branch
    GridSpan location;
};

struct ArcadeMatch {
    std::unique_ptr<Expression> challenger;
    struct Round {
        std::unique_ptr<Pattern> pattern;
        std::optional<std::unique_ptr<Expression>> portal_guard;  // if condition
        std::unique_ptr<Expression> consequence;
        GridSpan location;
    };
    std::vector<Round> rounds;
    GridSpan location;
};

struct BlockPortal {
    std::vector<std::unique_ptr<Statement>> transmissions;
    std::optional<std::unique_ptr<Expression>> final_pulse;  // implicit return
    GridSpan location;
};

struct LoopForever {
    std::unique_ptr<Expression> dimension;  // the loop body
    GridSpan location;
};

struct LoopWhile {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> dimension;
    GridSpan location;
};

struct ChaseLoop {
    std::unique_ptr<Pattern> tracker;
    std::unique_ptr<Expression> pursuit;  // iterator
    std::unique_ptr<Expression> dimension;
    GridSpan location;
};

struct BounceBack {
    std::optional<std::unique_ptr<Expression>> payload;  // return value
    GridSpan location;
};

struct BailOut {
    std::optional<std::unique_ptr<Expression>> payload;  // break value
    GridSpan location;
};

struct CruiseOn {
    GridSpan location;  // continue statement
};

struct RideWave {
    std::unique_ptr<Expression> async_future;  // await expr
    GridSpan location;
};

struct QuantumFlip {
    struct Parameter {
        std::unique_ptr<Pattern> pattern;
        std::optional<std::unique_ptr<TypeForm>> energy_form;
        GridSpan location;
    };
    std::vector<Parameter> inputs;
    std::optional<std::unique_ptr<TypeForm>> bounce_form;  // return type
    std::unique_ptr<Expression> manifestation;  // body
    bool is_turbo;  // async closure
    GridSpan location;
};

struct EntitySpawn {
    DataPath blueprint;
    struct SlotInit {
        Handle name;
        std::unique_ptr<Expression> value;
        GridSpan location;
    };
    std::vector<SlotInit> initialization;
    std::optional<std::unique_ptr<Expression>> base_entity;  // for struct update
    GridSpan location;
};

struct ClusterBuild {
    std::vector<std::unique_ptr<Expression>> elements;
    GridSpan location;
};

struct PathResolve {
    DataPath path;
    GridSpan location;
};

struct Expression {
    std::variant<
        // Literals - The Constants
        NumberGlow,
        FloatWave,
        TextBeam,
        GlyphAtom,
        TruthValue,
        SequenceLiteral,
        GridLiteral,
        // Operations - The Transformations
        BinaryBlast,
        UnaryTransform,
        CallBlast,
        MethodInvoke,
        FieldReach,
        IndexDive,
        MorphCast,
        QuantumTry,
        PipeFlow,
        // Control Flow - Navigate the Grid
        PortalBranch,
        ArcadeMatch,
        BlockPortal,
        LoopForever,
        LoopWhile,
        ChaseLoop,
        BounceBack,
        BailOut,
        CruiseOn,
        // Async - Turbo Mode
        RideWave,
        // Other Constructs
        QuantumFlip,
        EntitySpawn,
        ClusterBuild,
        PathResolve
    > essence;

    GridSpan location() const;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  📡 STATEMENTS - ACTIONS IN TIME                              ║
// ╚══════════════════════════════════════════════════════════════╝

struct WireStatement {
    std::unique_ptr<Pattern> binding;
    std::optional<std::unique_ptr<TypeForm>> energy_form;
    std::optional<std::unique_ptr<Expression>> initialization;
    GridSpan location;
};

struct ExpressionPulse {
    std::unique_ptr<Expression> signal;
    GridSpan location;
};

struct Statement {
    std::variant<
        WireStatement,
        ExpressionPulse
    > form;

    GridSpan location() const;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  🎮 DECLARATIONS - BUILDING THE WORLD                         ║
// ╚══════════════════════════════════════════════════════════════╝

struct PowerRune {
    Handle rune_name;
    std::vector<std::unique_ptr<Expression>> mystical_args;
    GridSpan location;
};

struct WaveParameter {
    std::unique_ptr<Pattern> binding;
    std::unique_ptr<TypeForm> energy_form;
    std::optional<std::unique_ptr<Expression>> default_charge;
    GridSpan location;
};

struct EnergyParameter {
    Handle name;
    std::vector<DataPath> protocol_bounds;  // trait bounds
    GridSpan location;
};

struct WaveDeclaration {
    std::vector<PowerRune> runes;
    Handle wave_name;
    std::vector<EnergyParameter> generic_params;
    std::vector<WaveParameter> inputs;
    std::optional<std::unique_ptr<TypeForm>> bounce_form;
    std::unique_ptr<Expression> manifestation;  // body
    bool is_broadcast;  // public
    bool is_turbo;      // async
    GridSpan location;
};

struct EntitySlot {
    std::vector<PowerRune> runes;
    Handle slot_name;
    std::unique_ptr<TypeForm> energy_form;
    bool is_broadcast;
    GridSpan location;
};

struct EntityDeclaration {
    std::vector<PowerRune> runes;
    Handle entity_name;
    std::vector<EnergyParameter> generic_params;
    std::variant<
        std::vector<EntitySlot>,                   // record entity { x: T, y: U }
        std::vector<std::unique_ptr<TypeForm>>,    // tuple entity(T, U);
        std::monostate                             // unit entity;
    > structure;
    bool is_broadcast;
    GridSpan location;
};

struct VariantChoice {
    std::vector<PowerRune> runes;
    Handle choice_name;
    std::variant<
        std::monostate,                           // unit variant
        std::vector<std::unique_ptr<TypeForm>>,   // tuple variant(T, U)
        std::vector<EntitySlot>                   // struct variant { x: T }
    > payload;
    GridSpan location;
};

struct VariantDeclaration {
    std::vector<PowerRune> runes;
    Handle variant_name;
    std::vector<EnergyParameter> generic_params;
    std::vector<VariantChoice> choices;
    bool is_broadcast;
    GridSpan location;
};

struct PowersDeclaration {
    std::vector<EnergyParameter> generic_params;
    std::optional<DataPath> protocol_path;  // for "Protocol for Type"
    std::unique_ptr<TypeForm> target_form;
    std::vector<std::unique_ptr<WaveDeclaration>> abilities;
    GridSpan location;
};

struct ProtocolDeclaration {
    std::vector<PowerRune> runes;
    Handle protocol_name;
    std::vector<EnergyParameter> generic_params;
    std::vector<std::unique_ptr<WaveDeclaration>> required_waves;  // signatures
    bool is_broadcast;
    GridSpan location;
};

struct StaticDeclaration {
    Handle constant_name;
    std::unique_ptr<TypeForm> energy_form;
    std::unique_ptr<Expression> frozen_value;
    bool is_broadcast;
    GridSpan location;
};

struct TypeWarp {
    Handle alias_name;
    std::vector<EnergyParameter> generic_params;
    std::unique_ptr<TypeForm> actual_form;
    bool is_broadcast;
    GridSpan location;
};

struct PlugDeclaration {
    struct PlugItem {
        Handle original_name;
        std::optional<Handle> morphed_name;  // as alias
    };

    std::variant<
        DataPath,                    // plug grid::database
        std::vector<PlugItem>        // plug { a, b morph c } cassette path
    > specification;
    DataPath source_path;
    GridSpan location;
};

struct BeamDeclaration {
    std::vector<Handle> broadcasts;  // what we're exporting
    GridSpan location;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  📼 TOP-LEVEL ITEMS AND CASSETTES                             ║
// ╚══════════════════════════════════════════════════════════════╝

struct ModuleItem {
    std::variant<
        std::unique_ptr<PlugDeclaration>,
        std::unique_ptr<BeamDeclaration>,
        std::unique_ptr<WaveDeclaration>,
        std::unique_ptr<EntityDeclaration>,
        std::unique_ptr<VariantDeclaration>,
        std::unique_ptr<PowersDeclaration>,
        std::unique_ptr<ProtocolDeclaration>,
        std::unique_ptr<StaticDeclaration>,
        std::unique_ptr<TypeWarp>
    > essence;

    GridSpan location() const;
};

struct Cassette {
    std::string cassette_path;  // file path
    std::vector<PowerRune> runes;
    std::vector<ModuleItem> items;
    GridSpan location;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  🌊 VISITOR PATTERN - TRAVERSE THE GRID                       ║
// ╚══════════════════════════════════════════════════════════════╝

class GridNavigator {
public:
    virtual ~GridNavigator() = default;

    // Traverse cassettes and items
    virtual void navigate_cassette(const Cassette& cassette) = 0;
    virtual void navigate_wave(const WaveDeclaration& wave) = 0;
    virtual void navigate_entity(const EntityDeclaration& entity) = 0;
    virtual void navigate_variant(const VariantDeclaration& variant) = 0;
    virtual void navigate_powers(const PowersDeclaration& powers) = 0;
    virtual void navigate_protocol(const ProtocolDeclaration& protocol) = 0;

    // Traverse expressions and patterns
    virtual void navigate_expression(const Expression& expr) = 0;
    virtual void navigate_statement(const Statement& stmt) = 0;
    virtual void navigate_pattern(const Pattern& pattern) = 0;
    virtual void navigate_type(const TypeForm& type) = 0;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║  🎨 UTILITY FUNCTIONS - DEBUGGING THE GRID                    ║
// ╚══════════════════════════════════════════════════════════════╝

/**
 * Render the AST in beautiful neon-colored text for debugging
 * (When output to terminal with ANSI support, keywords glow!)
 */
std::string render_neon_ast(const Cassette& cassette);
std::string render_neon_ast(const Expression& expr);
std::string render_neon_ast(const TypeForm& type);

/**
 * Compact S-expression format for unit tests
 */
std::string dump_sexpr(const Expression& expr);
std::string dump_sexpr(const TypeForm& type);

/**
 * JSON format for tooling integration
 */
std::string dump_json(const Cassette& cassette);

} // namespace neonsignal::neonecho::ast


// ╔══════════════════════════════════════════════════════════════╗
// ║  🌴 CLOSING TRANSMISSION                                      ║
// ╚══════════════════════════════════════════════════════════════╝
//
//  This is not just an AST.
//  This is a PORTAL into the GRID.
//
//  Every node carries the neon glow of the future.
//  Every traversal is a ride through electric dreams.
//
//  Now let's build the parser and MATERIALIZE this vision.
//
//  🌴⚡🎮 RIDE THE WAVE 🌴⚡🎮
//
