/**
 * The Lost Temple - Branching Text Adventure for Flipper Zero
 * v2.0 — Expanded with full NPC dialogue trees
 *
 * Characters:
 *   Edren    — hermit & former temple keeper (10+ dialogue nodes)
 *   Arek     — ghost of a fallen explorer    (5 dialogue nodes)
 *   Sentinel — the stone guardian you can negotiate with (5 nodes)
 *
 * Controls:
 *   UP / DOWN — move cursor between choices
 *   OK        — confirm choice
 *   BACK      — quit to main menu
 *
 * 42 scenes · 8 endings · 3 full NPC conversation trees
 *
 * Build:
 *   Place lost_temple.c + application.fam in
 *   applications_user/lost_temple/
 *   ./fbt fap_lost_temple
 *   Copy .fap to /ext/apps/Games/ on your Flipper.
 */

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <string.h>

/* ── Scene data ──────────────────────────────────────────────────────────── */

#define MAX_LINES   3
#define SCENE_COUNT 42

typedef struct {
    const char* title;
    const char* lines[MAX_LINES];
    const char* opt_a;
    const char* opt_b;
    uint8_t     next_a;
    uint8_t     next_b;
} Scene;

/*
 * ================================================================
 *  SCENE MAP
 * ================================================================
 *
 *  CORE PATH (0-17)
 *   0  Start
 *   1  Temple entrance
 *   2  Dark forest
 *   3  Left corridor — ghost appears first
 *   4  Right corridor — Sentinel
 *   5  Forest hermit — first greeting
 *   6  Trap disarmed
 *   7  Trap triggered
 *   8  Battle the Sentinel
 *   9  Sneak past Sentinel
 *  10  ENDING: peaceful forest life
 *  11  Inner chamber — the idol
 *  12  ENDING: cursed treasure
 *  13  ENDING: wisdom earned
 *  14  Hidden vault
 *  15  ENDING: fallen hero
 *  16  ENDING: wise retreat
 *  17  ENDING: the heir
 *
 *  AREK GHOST DIALOGUE (18-21, 36)
 *  18  Ghost of Arek appears
 *  19  Arek responds to kindness
 *  20  Arek angered by rudeness
 *  21  Arek's story — how he died
 *  36  Arek rests in peace — blesses you
 *
 *  SENTINEL DIALOGUE (22-25, 37)
 *  22  Speak to the Sentinel
 *  23  Sentinel: knowledge question
 *  24  Sentinel: angered by greed
 *  25  Sentinel bows — lets you pass
 *  37  Hidden alcove revealed
 *
 *  EDREN HERMIT DIALOGUE (26-35, 38)
 *  26  Edren: his name
 *  27  Edren: his regret
 *  28  Edren: temple secrets
 *  29  Edren: breaking the curse
 *  30  Edren won't go — gives charm
 *  31  Edren: about the Sentinel
 *  32  Edren: about the idol
 *  33  Edren's charm — warm light
 *  34  Edren's warning about greed
 *  35  Edren moved — you have a good heart
 *  38  Edren: the idol's origin
 *
 *  JOURNAL ENDING PATH (39-41)
 *  39  Edren's journal — read in alcove
 *  40  Chamber: armed with the truth
 *  41  ENDING: the truest hero
 */

static const Scene scenes[SCENE_COUNT] = {

    /* ─── 0: START ──────────────────────────────────────────────── */
    {
        "The Lost Temple",
        {"You wake in a cold", "forest. A stone temple", "looms through the fog."},
        "Enter the temple",
        "Explore the forest",
        1, 2
    },

    /* ─── 1: TEMPLE ENTRANCE ─────────────────────────────────────── */
    {
        "Temple Entrance",
        {"Two dark corridors", "gape before you.", "Torches flicker low."},
        "Take the left hall",
        "Take the right hall",
        3, 4
    },

    /* ─── 2: DARK FOREST ─────────────────────────────────────────── */
    {
        "Dark Forest",
        {"Twisted roots. Bird-", "song. A faint path winds", "into the dark."},
        "Head to the temple",
        "Follow the path",
        1, 5
    },

    /* ─── 3: LEFT CORRIDOR ───────────────────────────────────────── */
    {
        "Left Corridor",
        {"A silver plate gleams", "ahead — then a shimmer.", "A ghost drifts out."},
        "Speak to the ghost",
        "Try disarming trap",
        18, 6
    },

    /* ─── 4: RIGHT CORRIDOR ──────────────────────────────────────── */
    {
        "Right Corridor",
        {"A stone colossus", "blocks the path, red eyes", "scanning the dark."},
        "Speak to it",
        "Sneak around it",
        22, 9
    },

    /* ─── 5: HERMIT — GREETING ───────────────────────────────────── */
    {
        "Forest Hermit",
        {"A hooded elder tends", "a small fire. He looks", "up and nods at you."},
        "\"Who are you?\"",
        "\"What is in there?\"",
        26, 28
    },

    /* ─── 6: TRAP DISARMED ───────────────────────────────────────── */
    {
        "Trap Disarmed",
        {"Steady hands work the", "mechanism. A soft click.", "The path is clear."},
        "Continue forward",
        "Return to entrance",
        11, 1
    },

    /* ─── 7: TRAP SPRINGS ────────────────────────────────────────── */
    {
        "The Trap Springs!",
        {"You leap — but clip the", "plate. A spike grazes your", "shoulder. You bleed."},
        "Push through the pain",
        "Retreat to entrance",
        11, 1
    },

    /* ─── 8: BATTLE ──────────────────────────────────────────────── */
    {
        "Battle!",
        {"The Sentinel swings a", "stone fist. You roll and", "slash at its knee."},
        "Press the advantage",
        "Fall back — too strong!",
        11, 16
    },

    /* ─── 9: SNEAK PAST ──────────────────────────────────────────── */
    {
        "Into the Shadows",
        {"Hugging the cold wall", "you slip past the giant,", "unseen. Hall is clear."},
        "Move forward",
        "Search for hidden door",
        11, 14
    },

    /* ─── 10: ENDING — FOREST LIFE ───────────────────────────────── */
    {
        "END: A Gentle Life",
        {"You stay with Edren,", "learning the old ways.", "Peace finds you. END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ─── 11: INNER CHAMBER ──────────────────────────────────────── */
    {
        "Inner Chamber",
        {"A golden idol throbs", "with dark energy on an", "obsidian altar."},
        "Reach for the idol",
        "Read the runes instead",
        12, 13
    },

    /* ─── 12: ENDING — CURSED ────────────────────────────────────── */
    {
        "END: Cursed!",
        {"The idol floods your", "mind. Walls shake. You flee", "— but changed. THE END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ─── 13: ENDING — WISDOM ────────────────────────────────────── */
    {
        "END: True Wisdom",
        {"You sketch the runes", "carefully. The temple", "bows as you leave. END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ─── 14: HIDDEN VAULT ───────────────────────────────────────── */
    {
        "Hidden Vault",
        {"A stone door grinds", "open. Inside: ancient", "scrolls glow faintly."},
        "Read the scrolls",
        "Press on to chamber",
        17, 11
    },

    /* ─── 15: ENDING — FALLEN ────────────────────────────────────── */
    {
        "END: Fallen Hero",
        {"The Sentinel was too", "strong. You fall, but your", "story lives on. END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ─── 16: ENDING — WISE RETREAT ──────────────────────────────── */
    {
        "END: Lived to Fight",
        {"You withdraw, bruised", "but alive. Courage means", "knowing limits. END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ─── 17: ENDING — THE HEIR ──────────────────────────────────── */
    {
        "END: The True Heir",
        {"The scrolls reveal a", "pact — you are the heir", "to this temple. END"},
        "Play again",
        NULL,
        0, 0
    },

    /* ════════════════════════════════════════════════════════════════
     *  AREK — GHOST DIALOGUE  (scenes 18-21, 36)
     * ════════════════════════════════════════════════════════════════ */

    /* ─── 18: GHOST OF AREK APPEARS ─────────────────────────────── */
    {
        "Ghost of Arek",
        {"A pale specter drifts", "from the wall, blocking", "your path. It stares."},
        "Speak to it gently",
        "\"Step aside, ghost.\"",
        19, 20
    },

    /* ─── 19: AREK RESPONDS TO KINDNESS ─────────────────────────── */
    {
        "Arek: \"My Name\"",
        {"\"Few ever asked,\" it", "rasps. \"I am Arek. I", "died right here, see.\""},
        "\"How did you die?\"",
        "\"Show me safe path.\"",
        21, 6
    },

    /* ─── 20: AREK ANGERED ───────────────────────────────────────── */
    {
        "Arek: Angered",
        {"His eyes blaze white.", "\"Manners, child! Ask me", "nicely. Or walk alone.\""},
        "\"I'm sorry. Please help.\"",
        "Try the trap yourself",
        19, 7
    },

    /* ─── 21: AREK'S STORY ───────────────────────────────────────── */
    {
        "Arek: \"My Story\"",
        {"\"I was greedy once.", "I ran across that plate.", "The spikes... I stayed.\""},
        "\"I'll carry your tale.\"",
        "\"Is there a safe way?\"",
        36, 6
    },

    /* ════════════════════════════════════════════════════════════════
     *  SENTINEL — GUARDIAN DIALOGUE  (scenes 22-25, 37)
     * ════════════════════════════════════════════════════════════════ */

    /* ─── 22: SPEAK TO SENTINEL ──────────────────────────────────── */
    {
        "The Sentinel Speaks",
        {"Stone grinds on stone.", "The colossus tilts its head.", "A rumble: 'WHY HERE?'"},
        "\"I seek knowledge.\"",
        "\"I seek the gold idol.\"",
        23, 24
    },

    /* ─── 23: SENTINEL — KNOWLEDGE ──────────────────────────────── */
    {
        "Sentinel: \"Worthy?\"",
        {"\"Knowledge is worthy,\"", "it booms. \"Then answer:", "What truly guards truth?\""},
        "\"Patience & humility\"",
        "\"Strength and iron will\"",
        25, 8
    },

    /* ─── 24: SENTINEL — GREED ───────────────────────────────────── */
    {
        "Sentinel: \"Greed!\"",
        {"Its eyes blaze red.", "\"Greed defiles this hall.", "Prove you are more!\""},
        "\"I spoke rashly. Wisdom.\"",
        "\"Stand aside! I'll take it!\"",
        23, 8
    },

    /* ─── 25: SENTINEL BOWS ──────────────────────────────────────── */
    {
        "Sentinel Bows",
        {"The colossus steps", "aside, great head bowed low.", "'Seeker. You may pass.'"},
        "Thank the Sentinel",
        "Walk past quickly",
        37, 11
    },

    /* ════════════════════════════════════════════════════════════════
     *  EDREN — HERMIT DIALOGUE  (scenes 26-35, 38)
     * ════════════════════════════════════════════════════════════════ */

    /* ─── 26: EDREN — HIS NAME ───────────────────────────────────── */
    {
        "Edren: \"My Name\"",
        {"\"They called me Edren,\"", "he says. \"I once kept", "this temple. Long ago.\""},
        "\"Why did you leave?\"",
        "\"What is inside now?\"",
        27, 28
    },

    /* ─── 27: EDREN — REGRET ─────────────────────────────────────── */
    {
        "Edren: His Regret",
        {"\"The idol corrupted", "my order. I fled. I have", "been ashamed ever since.\""},
        "\"Can the curse be broken?\"",
        "\"Please come with me.\"",
        29, 30
    },

    /* ─── 28: EDREN — TEMPLE SECRETS ────────────────────────────── */
    {
        "Edren: The Temple",
        {"\"A guardian walks the", "right hall. A ghost and", "trap guard the left.\""},
        "\"How face the guardian?\"",
        "\"Tell me of the idol.\"",
        31, 32
    },

    /* ─── 29: EDREN — BREAKING THE CURSE ────────────────────────── */
    {
        "Edren: The Curse",
        {"\"Inscribe the runes", "below the altar. Do NOT", "touch the idol itself.\""},
        "\"Thank you, Edren.\"",
        "\"What if I take it?\"",
        33, 34
    },

    /* ─── 30: EDREN WON'T GO — GIVES CHARM ──────────────────────── */
    {
        "Edren: \"I Can't Go\"",
        {"\"It would know me. But", "take this—\" He presses", "a warm charm to you."},
        "\"Goodbye, Edren.\"",
        "\"I need you there.\"",
        1, 35
    },

    /* ─── 31: EDREN — ABOUT THE SENTINEL ────────────────────────── */
    {
        "Edren: The Sentinel",
        {"\"Speak to it first.", "They were scholars once,", "Ask its true purpose.\""},
        "\"Fascinating. Thank you.\"",
        "\"And the left corridor?\"",
        1, 28
    },

    /* ─── 32: EDREN — ABOUT THE IDOL ────────────────────────────── */
    {
        "Edren: The Idol",
        {"\"It whispers promises.", "Each one is real — but the", "cost is your whole self.\""},
        "\"I'll be careful.\"",
        "\"Who made it?\"",
        1, 38
    },

    /* ─── 33: EDREN'S CHARM ──────────────────────────────────────── */
    {
        "Edren's Charm",
        {"Amber light glows from", "the amulet. You feel", "protected. Time to go."},
        "Enter the temple",
        NULL,
        1, 0
    },

    /* ─── 34: EDREN'S WARNING ────────────────────────────────────── */
    {
        "Edren: \"Lost Already\"",
        {"\"Then you are already", "lost,\" Edren says,", "turning slowly away."},
        "\"I was joking. Careful.\"",
        "\"Farewell, old man.\"",
        33, 1
    },

    /* ─── 35: EDREN MOVED ────────────────────────────────────────── */
    {
        "Edren: \"Good Heart\"",
        {"\"You have a good heart,\"", "Edren smiles. \"Return", "and tell me everything.\""},
        "\"I promise, Edren.\"",
        "Stay in the forest",
        1, 10
    },

    /* ─── 36: AREK RESTS IN PEACE ───────────────────────────────── */
    {
        "Arek: At Peace",
        {"Arek smiles — first", "smile in a century. He", "fades. You feel blessed."},
        "Walk to the chamber",
        NULL,
        11, 0
    },

    /* ─── 37: HIDDEN ALCOVE (from Sentinel bow) ─────────────────── */
    {
        "The Hidden Alcove",
        {"The Sentinel's bow", "reveals a carved alcove.", "A journal rests within."},
        "Read the journal",
        "Take the back door",
        39, 11
    },

    /* ─── 38: EDREN — IDOL'S ORIGIN ─────────────────────────────── */
    {
        "Edren: \"The Origin\"",
        {"\"The first priests made", "it as a test. To fail", "the test is to take it.\""},
        "\"I understand now.\"",
        "\"I will pass the test.\"",
        33, 1
    },

    /* ════════════════════════════════════════════════════════════════
     *  JOURNAL ENDING PATH  (scenes 39-41)
     * ════════════════════════════════════════════════════════════════ */

    /* ─── 39: EDREN'S JOURNAL ────────────────────────────────────── */
    {
        "Edren's Journal",
        {"Edren's handwriting:", "'The idol's true power", "is in refusing it.'"},
        "Go to inner chamber",
        NULL,
        40, 0
    },

    /* ─── 40: CHAMBER — ARMED WITH TRUTH ────────────────────────── */
    {
        "Chamber: The Truth",
        {"The idol pulses and", "whispers: 'Take me. You've", "earned it.' You know better."},
        "Walk away. Leave it.",
        "Take it — you earned it.",
        41, 12
    },

    /* ─── 41: ENDING — THE TRUEST HERO ──────────────────────────── */
    {
        "END: The Truest Hero",
        {"You leave the idol.", "The temple shudders, glows.", "You step into light.END"},
        "Play again",
        NULL,
        0, 0
    },

}; /* end scenes[] */

/* ── App state ───────────────────────────────────────────────────────────── */

typedef struct {
    uint8_t scene;
    uint8_t cursor;
} GameState;

typedef struct {
    GameState         state;
    FuriMutex*        mutex;
    FuriMessageQueue* queue;
    ViewPort*         viewport;
    Gui*              gui;
} App;

/* ── Drawing ─────────────────────────────────────────────────────────────── */

static void draw_callback(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    const GameState* gs    = &app->state;
    const Scene*     scene = &scenes[gs->scene];

    canvas_clear(canvas);

    /* Title bar */
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, scene->title);
    canvas_draw_line(canvas, 0, 11, 127, 11);

    /* Story lines */
    canvas_set_font(canvas, FontSecondary);
    for(int i = 0; i < MAX_LINES; i++) {
        if(scene->lines[i] && scene->lines[i][0] != '\0') {
            canvas_draw_str(canvas, 2, 21 + i * 10, scene->lines[i]);
        }
    }

    /* Choices */
    if(scene->opt_b) {
        /* Two choices */
        canvas_draw_line(canvas, 0, 48, 127, 48);
        if(gs->cursor == 0) canvas_draw_str(canvas, 1, 56, "\x7e");
        canvas_draw_str(canvas, 9, 56, scene->opt_a);
        if(gs->cursor == 1) canvas_draw_str(canvas, 1, 63, "\x7e");
        canvas_draw_str(canvas, 9, 63, scene->opt_b);
    } else if(scene->opt_a) {
        /* Single choice */
        canvas_draw_line(canvas, 0, 50, 127, 50);
        canvas_draw_str(canvas, 1, 60, "\x7e");
        canvas_draw_str(canvas, 9, 60, scene->opt_a);
    }

    furi_mutex_release(app->mutex);
}

/* ── Input ───────────────────────────────────────────────────────────────── */

static void input_callback(InputEvent* event, void* ctx) {
    App* app = ctx;
    furi_message_queue_put(app->queue, event, FuriWaitForever);
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

int32_t lost_temple_app(void* p) {
    UNUSED(p);

    App* app   = malloc(sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    app->state.scene  = 0;
    app->state.cursor = 0;

    app->viewport = view_port_alloc();
    view_port_draw_callback_set(app->viewport, draw_callback, app);
    view_port_input_callback_set(app->viewport, input_callback, app);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->viewport, GuiLayerFullscreen);

    InputEvent event;
    bool       running = true;

    while(running) {
        if(furi_message_queue_get(app->queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort || event.type == InputTypeRepeat) {
                furi_mutex_acquire(app->mutex, FuriWaitForever);

                const Scene* scene = &scenes[app->state.scene];

                switch(event.key) {
                case InputKeyBack:
                    running = false;
                    break;
                case InputKeyUp:
                    app->state.cursor = 0;
                    break;
                case InputKeyDown:
                    if(scene->opt_b != NULL) app->state.cursor = 1;
                    break;
                case InputKeyOk:
                    if(app->state.cursor == 0) {
                        app->state.scene  = scene->next_a;
                        app->state.cursor = 0;
                    } else if(app->state.cursor == 1 && scene->opt_b != NULL) {
                        app->state.scene  = scene->next_b;
                        app->state.cursor = 0;
                    }
                    break;
                default:
                    break;
                }

                furi_mutex_release(app->mutex);
                view_port_update(app->viewport);
            }
        }
    }

    gui_remove_view_port(app->gui, app->viewport);
    furi_record_close(RECORD_GUI);
    view_port_free(app->viewport);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
