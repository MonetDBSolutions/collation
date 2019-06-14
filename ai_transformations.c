#include <monetdb_config.h>
#include <mal_exception.h>

#include <gdk.h>
#include <gdk_atoms.h>

#include <string.h>

/* we use the PCRE library to do regular expression matching */
#include <pcre.h>

extern __declspec(dllexport) char *UDFget_re(char ** result, const char **input);
extern __declspec(dllexport) char *UDFnormalize(char ** result, const char **input);
extern __declspec(dllexport) char *UDFBATnormalize(bat *result, const bat *input);

#define A_CLASS "[A\303\200\303\201\303\202\303\203\303\204\303\205\304\200\304\202\304\204\307\215\307\236\307\240\307\272\310\200\310\202\310\246\310\272\341\270\200\341\272\240\341\272\242\341\272\244\341\272\246\341\272\250\341\272\252\341\272\254\341\272\256\341\272\260\341\272\262\341\272\264\341\272\266a\303\240\303\241\303\242\303\243\303\244\303\245\304\201\304\203\304\205\307\216\307\237\307\241\307\273\310\201\310\203\310\247\341\266\217\341\270\201\341\272\232\341\272\241\341\272\243\341\272\245\341\272\247\341\272\251\341\272\253\341\272\255\341\272\257\341\272\261\341\272\263\341\272\265\341\272\267\342\261\245\352\254\261][\314\200-\315\257]*"
#define B_CLASS "[B\306\201\306\202\311\203\341\270\202\341\270\204\341\270\206\352\236\226b\306\200\306\203\311\223\341\265\254\341\266\200\341\270\203\341\270\205\341\270\207\352\236\227][\314\200-\315\257]*"
#define C_CLASS "[C\303\207\304\206\304\210\304\212\304\214\306\207\310\273\341\270\210\352\236\222c\303\247\304\207\304\211\304\213\304\215\306\210\310\274\311\225\341\270\211\352\236\223\352\236\224][\314\200-\315\257]*"
#define D_CLASS "[D\304\216\304\220\306\212\306\213\307\205\307\262\341\270\212\341\270\214\341\270\216\341\270\220\341\270\222d\304\217\304\221\306\214\310\241\311\226\311\227\341\265\255\341\266\201\341\266\221\341\270\213\341\270\215\341\270\217\341\270\221\341\270\223][\314\200-\315\257]*"
#define E_CLASS "[E\303\210\303\211\303\212\303\213\304\222\304\224\304\226\304\230\304\232\310\204\310\206\310\250\311\206\341\270\224\341\270\226\341\270\230\341\270\232\341\270\234\341\272\270\341\272\272\341\272\274\341\272\276\341\273\200\341\273\202\341\273\204\341\273\206e\303\250\303\251\303\252\303\253\304\223\304\225\304\227\304\231\304\233\310\205\310\207\310\251\311\207\341\266\222\341\270\225\341\270\227\341\270\231\341\270\233\341\270\235\341\272\271\341\272\273\341\272\275\341\272\277\341\273\201\341\273\203\341\273\205\341\273\207\342\261\270\352\254\264][\314\200-\315\257]*"
#define F_CLASS "[F\306\221\341\270\236\352\236\230f\306\222\341\265\256\341\266\202\341\270\237\352\236\231][\314\200-\315\257]*"
#define G_CLASS "[G\304\234\304\236\304\240\304\242\306\223\307\244\307\246\307\264\341\270\240\352\236\240g\304\235\304\237\304\241\304\243\307\245\307\247\307\265\311\240\341\266\203\341\270\241\352\236\241][\314\200-\315\257]*"
#define H_CLASS "[H\304\244\304\246\310\236\341\270\242\341\270\244\341\270\246\341\270\250\341\270\252\342\261\247\352\236\252h\304\245\304\247\310\237\311\246\341\270\243\341\270\245\341\270\247\341\270\251\341\270\253\341\272\226\342\261\250\352\236\225][\314\200-\315\257]*"
#define I_CLASS "[I\303\214\303\215\303\216\303\217\304\250\304\252\304\254\304\256\304\260\306\227\307\217\310\210\310\212\341\270\254\341\270\256\341\273\210\341\273\212i\303\254\303\255\303\256\303\257\304\251\304\253\304\255\304\257\307\220\310\211\310\213\311\250\341\266\226\341\270\255\341\270\257\341\273\211\341\273\213][\314\200-\315\257]*"
#define J_CLASS "[J\304\264\311\210\352\236\262j\304\265\307\260\311\211\312\235][\314\200-\315\257]*"
#define K_CLASS "[K\304\266\306\230\307\250\341\270\260\341\270\262\341\270\264\342\261\251\352\235\200\352\235\202\352\235\204\352\236\242k\304\267\306\231\307\251\341\266\204\341\270\261\341\270\263\341\270\265\342\261\252\352\235\201\352\235\203\352\235\205\352\236\243][\314\200-\315\257]*"
#define L_CLASS "[L\304\271\304\273\304\275\304\277\305\201\307\210\310\275\341\270\266\341\270\270\341\270\272\341\270\274\342\261\240\342\261\242\352\235\210\352\236\255l\304\272\304\274\304\276\305\200\305\202\306\232\310\264\311\253\311\254\311\255\341\266\205\341\270\267\341\270\271\341\270\273\341\270\275\342\261\241\352\235\211\352\236\216\352\254\267\352\254\270\352\254\271][\314\200-\315\257]*"
#define M_CLASS "[M\341\270\276\341\271\200\341\271\202\342\261\256m\311\261\341\265\257\341\266\206\341\270\277\341\271\201\341\271\203\352\254\272][\314\200-\315\257]*"
#define N_CLASS "[N\303\221\305\203\305\205\305\207\306\235\307\213\307\270\310\240\341\271\204\341\271\206\341\271\210\341\271\212\352\236\220\352\236\244n\303\261\305\204\305\206\305\210\305\211\306\236\307\271\310\265\311\262\311\263\341\265\260\341\266\207\341\271\205\341\271\207\341\271\211\341\271\213\352\236\221\352\236\245\352\254\273][\314\200-\315\257]*"
#define O_CLASS "[O\303\222\303\223\303\224\303\225\303\226\303\230\305\214\305\216\305\220\306\237\306\240\307\221\307\252\307\254\307\276\310\214\310\216\310\252\310\254\310\256\310\260\341\271\214\341\271\216\341\271\220\341\271\222\341\273\214\341\273\216\341\273\220\341\273\222\341\273\224\341\273\226\341\273\230\341\273\232\341\273\234\341\273\236\341\273\240\341\273\242\352\235\212\352\235\214o\303\262\303\263\303\264\303\265\303\266\303\270\305\215\305\217\305\221\306\241\307\222\307\253\307\255\307\277\310\215\310\217\310\253\310\255\310\257\310\261\341\271\215\341\271\217\341\271\221\341\271\223\341\273\215\341\273\217\341\273\221\341\273\223\341\273\225\341\273\227\341\273\231\341\273\233\341\273\235\341\273\237\341\273\241\341\273\243\342\261\272\352\235\213\352\235\215][\314\200-\315\257]*"
#define P_CLASS "[P\306\244\341\271\224\341\271\226\342\261\243\352\235\220\352\235\222\352\235\224p\306\245\341\265\261\341\265\275\341\266\210\341\271\225\341\271\227\352\235\221\352\235\223\352\235\225][\314\200-\315\257]*"
#define Q_CLASS "[Q\352\235\226\352\235\230q\311\213\312\240\352\235\227\352\235\231][\314\200-\315\257]*"
#define R_CLASS "[R\305\224\305\226\305\230\310\220\310\222\311\214\341\271\230\341\271\232\341\271\234\341\271\236\342\261\244\352\235\232\352\236\246r\305\225\305\227\305\231\310\221\310\223\311\215\311\274\311\275\311\276\341\265\262\341\265\263\341\266\211\341\271\231\341\271\233\341\271\235\341\271\237\352\235\233\352\236\247\352\255\207\352\255\211][\314\200-\315\257]*"
#define S_CLASS "[S\305\232\305\234\305\236\305\240\310\230\341\271\240\341\271\242\341\271\244\341\271\246\341\271\250\342\261\276\352\236\250s\305\233\305\235\305\237\305\241\310\231\310\277\312\202\341\265\264\341\266\212\341\271\241\341\271\243\341\271\245\341\271\247\341\271\251\352\236\251][\314\200-\315\257]*"
#define T_CLASS "[T\305\242\305\244\305\246\306\254\306\256\310\232\310\276\341\271\252\341\271\254\341\271\256\341\271\260t\305\243\305\245\305\247\306\253\306\255\310\233\310\266\312\210\341\265\265\341\271\253\341\271\255\341\271\257\341\271\261\341\272\227\342\261\246][\314\200-\315\257]*"
#define U_CLASS "[U\303\231\303\232\303\233\303\234\305\250\305\252\305\254\305\256\305\260\305\262\306\257\307\223\307\225\307\227\307\231\307\233\310\224\310\226\311\204\341\271\262\341\271\264\341\271\266\341\271\270\341\271\272\341\273\244\341\273\246\341\273\250\341\273\252\341\273\254\341\273\256\341\273\260\352\236\270u\303\271\303\272\303\273\303\274\305\251\305\253\305\255\305\257\305\261\305\263\306\260\307\224\307\226\307\230\307\232\307\234\310\225\310\227\312\211\341\266\231\341\271\263\341\271\265\341\271\267\341\271\271\341\271\273\341\273\245\341\273\247\341\273\251\341\273\253\341\273\255\341\273\257\341\273\261\352\236\271\352\255\216\352\255\217\352\255\222][\314\200-\315\257]*"
#define V_CLASS "[V\306\262\341\271\274\341\271\276\352\235\236v\312\213\341\266\214\341\271\275\341\271\277\342\261\261\342\261\264\352\235\237][\314\200-\315\257]*"
#define W_CLASS "[W\305\264\341\272\200\341\272\202\341\272\204\341\272\206\341\272\210\342\261\262w\305\265\341\272\201\341\272\203\341\272\205\341\272\207\341\272\211\341\272\230\342\261\263][\314\200-\315\257]*"
#define X_CLASS "[X\341\272\212\341\272\214x\341\266\215\341\272\213\341\272\215\352\255\226\352\255\227\352\255\230\352\255\231][\314\200-\315\257]*"
#define Y_CLASS "[Y\303\235\305\266\305\270\306\263\310\262\311\216\341\272\216\341\273\262\341\273\264\341\273\266\341\273\270\341\273\276y\303\275\303\277\305\267\306\264\310\263\311\217\341\272\217\341\272\231\341\273\263\341\273\265\341\273\267\341\273\271\341\273\277\352\255\232][\314\200-\315\257]*"
#define Z_CLASS "[Z\305\271\305\273\305\275\306\265\310\244\341\272\220\341\272\222\341\272\224\342\261\253\342\261\277z\305\272\305\274\305\276\306\266\310\245\311\200\312\220\312\221\341\265\266\341\266\216\341\272\221\341\272\223\341\272\225\342\261\254][\314\200-\315\257]*"

#define MULTI_WILDCARD_CLASS "%"
#define MULTI_WILDCARD_REPLACEMENT ".*"

#define SINGLE_WILDCARD_CLASS "_"
#define SINGLE_WILDCARD_REPLACEMENT "."

typedef struct _character_class_t {
    const char* match;
    const char* replacement;
    size_t      replacement_length;
} character_class_t;

#define init_character_class(symbol, replacement) {symbol, replacement, strlen(replacement)}

#define NREG_EXPRS 28 // Latin alphabet plus the two wildcards.

static const character_class_t REG_EXPRS[NREG_EXPRS] = {
    init_character_class(MULTI_WILDCARD_CLASS, MULTI_WILDCARD_REPLACEMENT),
    init_character_class(SINGLE_WILDCARD_CLASS, SINGLE_WILDCARD_REPLACEMENT),
    init_character_class(A_CLASS, A_CLASS),
    init_character_class(B_CLASS, B_CLASS),
    init_character_class(C_CLASS, C_CLASS),
    init_character_class(D_CLASS, D_CLASS),
    init_character_class(E_CLASS, E_CLASS),
    init_character_class(F_CLASS, F_CLASS),
    init_character_class(G_CLASS, G_CLASS),
    init_character_class(H_CLASS, H_CLASS),
    init_character_class(I_CLASS, I_CLASS),
    init_character_class(J_CLASS, J_CLASS),
    init_character_class(K_CLASS, K_CLASS),
    init_character_class(L_CLASS, L_CLASS),
    init_character_class(M_CLASS, M_CLASS),
    init_character_class(N_CLASS, N_CLASS),
    init_character_class(O_CLASS, O_CLASS),
    init_character_class(P_CLASS, P_CLASS),
    init_character_class(Q_CLASS, Q_CLASS),
    init_character_class(R_CLASS, R_CLASS),
    init_character_class(S_CLASS, S_CLASS),
    init_character_class(T_CLASS, T_CLASS),
    init_character_class(U_CLASS, U_CLASS),
    init_character_class(V_CLASS, V_CLASS),
    init_character_class(W_CLASS, W_CLASS),
    init_character_class(X_CLASS, X_CLASS),
    init_character_class(Y_CLASS, Y_CLASS),
    init_character_class(Z_CLASS, Z_CLASS)
};

#define NNORM_EXPRS 26 // Just the Latin alphabet.

static const character_class_t NORM_EXPRS[NNORM_EXPRS] = {
    init_character_class(A_CLASS, "a"),
    init_character_class(B_CLASS, "b"),
    init_character_class(C_CLASS, "c"),
    init_character_class(D_CLASS, "d"),
    init_character_class(E_CLASS, "e"),
    init_character_class(F_CLASS, "f"),
    init_character_class(G_CLASS, "g"),
    init_character_class(H_CLASS, "h"),
    init_character_class(I_CLASS, "i"),
    init_character_class(J_CLASS, "j"),
    init_character_class(K_CLASS, "k"),
    init_character_class(L_CLASS, "l"),
    init_character_class(M_CLASS, "m"),
    init_character_class(N_CLASS, "n"),
    init_character_class(O_CLASS, "o"),
    init_character_class(P_CLASS, "p"),
    init_character_class(Q_CLASS, "q"),
    init_character_class(R_CLASS, "r"),
    init_character_class(S_CLASS, "s"),
    init_character_class(T_CLASS, "t"),
    init_character_class(U_CLASS, "u"),
    init_character_class(V_CLASS, "v"),
    init_character_class(W_CLASS, "w"),
    init_character_class(X_CLASS, "x"),
    init_character_class(Y_CLASS, "y"),
    init_character_class(Z_CLASS, "z")
};

#define ovector_length 3

typedef struct merge_element_t merge_element_t;

struct merge_element_t {
    int size;
    const character_class_t* class;
};

static inline void merge_array_insert(merge_element_t* merge_array, int offset, int size, const character_class_t* class) {
    merge_element_t* element = &merge_array[offset];
    element->size = size;
    element->class = class;
}

static inline void compile_patterns(pcre** compiled_patterns, const character_class_t* classes, int nclasses) {

    const char* err = NULL;
    int erroffset;

    for (int i = 0; i < nclasses; i ++) {
        const character_class_t* class = &classes[i];

        int options = PCRE_UTF8 | PCRE_MULTILINE | PCRE_DOTALL;
        compiled_patterns[i] = pcre_compile(class->match, options, &err, &erroffset, NULL);

        // PERHAPS we should study these expressions for the sake of UDFBATnormalize.

        if (err) {
            // TODO error handling
            printf("%s\n", err);
        }
    }
}

static inline void free_compiled_patterns(pcre* const * compiled_patterns, int nclasses) {

    for (int i = 0; i < nclasses; i ++) {
        pcre_free(compiled_patterns[i]);
    }
}

static void ai_transform(
    char** result_cursor,
    const char* input,
    pcre* const * compiled_patterns, const character_class_t* classes, int nclasses,
    merge_element_t* merge_array, int input_length) {

    int ovector[ovector_length];

    for (int i = 0; i < nclasses; i ++) {
        int pos = -1;
        int begin = 0;
        int end   = 0;

        while ( (pos = pcre_exec(compiled_patterns[i], NULL, input, input_length, end, 0, ovector, ovector_length)) > PCRE_ERROR_NOMATCH ) {
            begin = ovector[0];
            end   = ovector[1];
            int size = end - begin;
            const character_class_t* class = &classes[i];

            merge_array_insert(merge_array, begin, size, class);
        }
        // TODO handle pos < -1 aka errors
    }

    for (int i = 0; i < input_length;) {
        merge_element_t* merge_element = &merge_array[i];

        if (merge_element->size) {

            strncpy(*result_cursor, merge_element->class->replacement, merge_element->class->replacement_length);
            *result_cursor += merge_element->class->replacement_length;
            i+=merge_element->size;

            continue;
        }

        *(*result_cursor)++ = *(input + i++); // Just copy the current byte
    }

    **result_cursor = 0; // Finalize transformed_pat with null terminator
}

char *UDFget_re(char ** result, const char **input) {
    if (GDK_STRNIL(*input)) {

    // TODO: check for allocation errors.
        *result = GDKmalloc(sizeof(str_nil));
        strcpy (*result, str_nil);

	    return MAL_SUCCEED;
    }

    int pat_length =  strlen(*input);

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKzalloc(pat_length * sizeof(merge_element_t));

    pcre* compiled_patterns[NREG_EXPRS];
    compile_patterns(compiled_patterns, REG_EXPRS, NREG_EXPRS);

    // NOTE: this is an inordinately large buffer for typical usecases.
    int buffer_size = strlen(O_CLASS) * (pat_length + 2) +1;

    // TODO: check for allocation errors.
    *result = GDKzalloc(buffer_size);

    char* cursor = *result;
    *cursor++ = '^'; // match start of string.

    // TODO error handling
    ai_transform(&cursor, *input, compiled_patterns, REG_EXPRS, NREG_EXPRS, merge_array, pat_length);

    *cursor++ = '$'; // Overwrite null terminator and match end of string.

    *cursor = 0; // Finalize transformed_pat with null terminator.

    free_compiled_patterns(compiled_patterns, NREG_EXPRS);
    GDKfree(merge_array);

	return MAL_SUCCEED;
}

char *UDFnormalize(char ** result, const char **input) {
    if (GDK_STRNIL(*input)) {

    // TODO: check for allocation errors.
        *result = GDKmalloc(sizeof(str_nil));
        strcpy (*result, str_nil);

	    return MAL_SUCCEED;
    }

    int pat_length =  strlen(*input);

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKzalloc(pat_length * sizeof(merge_element_t));

    pcre* compiled_patterns[NNORM_EXPRS];
    compile_patterns(compiled_patterns, NORM_EXPRS, NNORM_EXPRS);

    int buffer_size = pat_length + 1;

    // TODO: check for allocation errors.
    *result = GDKzalloc(buffer_size);

    char* cursor = *result;

    // TODO error handling
    ai_transform(&cursor, *input, compiled_patterns, NORM_EXPRS, NNORM_EXPRS, merge_array, pat_length);

    free_compiled_patterns(compiled_patterns, NNORM_EXPRS);
    GDKfree(merge_array);

	return MAL_SUCCEED;
}

char *
UDFBATnormalize(bat *result, const bat *input) {
	BAT *input_bat, *result_bat;
	BATiter bi;
	BUN p, q;

	input_bat = BATdescriptor(*input);
	if (input_bat == NULL) {
		throw(MAL, "collation.pattern2normalized", RUNTIME_OBJECT_MISSING);
	}

	assert(input_bat->ttype == TYPE_str);

	result_bat = COLnew(input_bat->hseqbase, TYPE_str, BATcount(input_bat), TRANSIENT);
	if (result_bat == NULL) {
		BBPunfix(input_bat->batCacheid);
		throw(MAL, "collation.pattern2normalized", MAL_MALLOC_FAIL);
	}

    int array_size = 64;

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKmalloc(array_size * sizeof(merge_element_t));

    pcre* compiled_patterns[NNORM_EXPRS];
    compile_patterns(compiled_patterns, NORM_EXPRS, NNORM_EXPRS);

    // TODO: check for allocation errors.
    char* result_val = GDKmalloc(array_size + 1);

	bi = bat_iterator(input_bat);
	BATloop(input_bat, p, q) {
		const char *input_val = (const char *) BUNtail(bi, p);

		if (GDK_STRNIL(input_val)) {
            if (BUNappend(result_bat, str_nil, false) != GDK_SUCCEED) {
                // TODO error handling
            }

            continue;
		}

        const int source_length = strlen(input_val);

		if (source_length >= array_size) {

            array_size = 2 * source_length;

            merge_array = GDKrealloc(merge_array, array_size * sizeof(merge_element_t));

            result_val = GDKrealloc(result_val, array_size + 1);
		}

        memset(merge_array, 0, array_size * sizeof(merge_element_t));

        char* cursor = result_val;

        // TODO error handling
        ai_transform(&cursor, input_val, compiled_patterns, NORM_EXPRS, NNORM_EXPRS, merge_array, source_length);

		if (BUNappend(result_bat, result_val, false) != GDK_SUCCEED) {
			// TODO error handling
		}
	}

    GDKfree(result_val);
    free_compiled_patterns(compiled_patterns, NNORM_EXPRS);
	GDKfree(merge_array);
	BBPunfix(input_bat->batCacheid);

	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	return MAL_SUCCEED;
}
