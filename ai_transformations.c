#include <monetdb_config.h>
#include <mal_exception.h>

#include <gdk.h>
#include <gdk_atoms.h>

#include <string.h>

/* we use the PCRE library to do regular expression matching */
#include <pcre.h>

extern __declspec(dllexport) char *UDFpattern2re(char ** result, const char **input);
extern __declspec(dllexport) char *UDFpattern2normalized(char ** result, const char **input);
extern __declspec(dllexport) char *UDFBATpattern2normalized(bat *result, const bat *input);

#define A_CLASS "[A\\x{00C0}\\x{00C1}\\x{00C2}\\x{00C3}\\x{00C4}\\x{00C5}\\x{0100}\\x{0102}\\x{0104}\\x{01CD}\\x{01DE}\\x{01E0}\\x{01FA}\\x{0200}\\x{0202}\\x{0226}\\x{023A}\\x{1E00}\\x{1EA0}\\x{1EA2}\\x{1EA4}\\x{1EA6}\\x{1EA8}\\x{1EAA}\\x{1EAC}\\x{1EAE}\\x{1EB0}\\x{1EB2}\\x{1EB4}\\x{1EB6}a\\x{00E0}\\x{00E1}\\x{00E2}\\x{00E3}\\x{00E4}\\x{00E5}\\x{0101}\\x{0103}\\x{0105}\\x{01CE}\\x{01DF}\\x{01E1}\\x{01FB}\\x{0201}\\x{0203}\\x{0227}\\x{1D8F}\\x{1E01}\\x{1E9A}\\x{1EA1}\\x{1EA3}\\x{1EA5}\\x{1EA7}\\x{1EA9}\\x{1EAB}\\x{1EAD}\\x{1EAF}\\x{1EB1}\\x{1EB3}\\x{1EB5}\\x{1EB7}\\x{2C65}\\x{AB31}][\\x{0300}-\\x{036F}]*"
#define B_CLASS "[b\\x{0180}\\x{0183}\\x{0253}\\x{1D6C}\\x{1D80}\\x{1E03}\\x{1E05}\\x{1E07}\\x{A797}B\\x{0181}\\x{0182}\\x{0243}\\x{1E02}\\x{1E04}\\x{1E06}\\x{A796}][\\x{0300}-\\x{036F}]*"
#define C_CLASS "[C\\x{00C7}\\x{0106}\\x{0108}\\x{010A}\\x{010C}\\x{0187}\\x{023B}\\x{1E08}\\x{A792}c\\x{00E7}\\x{0107}\\x{0109}\\x{010B}\\x{010D}\\x{0188}\\x{023C}\\x{0255}\\x{1E09}\\x{A793}\\x{A794}][\\x{0300}-\\x{036F}]*"
#define D_CLASS "[D\\x{00D0}\\x{010E}\\x{0110}\\x{018A}\\x{018B}\\x{01C5}\\x{01F2}\\x{1E0A}\\x{1E0C}\\x{1E0E}\\x{1E10}\\x{1E12}d\\x{010F}\\x{0111}\\x{018C}\\x{0221}\\x{0256}\\x{0257}\\x{1D6D}\\x{1D81}\\x{1D91}\\x{1E0B}\\x{1E0D}\\x{1E0F}\\x{1E11}\\x{1E13}][\\x{0300}-\\x{036F}]*"
#define E_CLASS "[E\\x{00C8}\\x{00C9}\\x{00CA}\\x{00CB}\\x{0112}\\x{0114}\\x{0116}\\x{0118}\\x{011A}\\x{0204}\\x{0206}\\x{0228}\\x{0246}\\x{1E14}\\x{1E16}\\x{1E18}\\x{1E1A}\\x{1E1C}\\x{1EB8}\\x{1EBA}\\x{1EBC}\\x{1EBE}\\x{1EC0}\\x{1EC2}\\x{1EC4}\\x{1EC6}e\\x{00E8}\\x{00E9}\\x{00EA}\\x{00EB}\\x{0113}\\x{0115}\\x{0117}\\x{0119}\\x{011B}\\x{0205}\\x{0207}\\x{0229}\\x{0247}\\x{1D92}\\x{1E15}\\x{1E17}\\x{1E19}\\x{1E1B}\\x{1E1D}\\x{1EB9}\\x{1EBB}\\x{1EBD}\\x{1EBF}\\x{1EC1}\\x{1EC3}\\x{1EC5}\\x{1EC7}\\x{2C78}\\x{AB34}][\\x{0300}-\\x{036F}]*"
#define F_CLASS "[F\\x{0191}\\x{1E1E}\\x{A798}f\\x{0192}\\x{1D6E}\\x{1D82}\\x{1E1F}\\x{A799}][\\x{0300}-\\x{036F}]*"
#define G_CLASS "[G\\x{011C}\\x{011E}\\x{0120}\\x{0122}\\x{0193}\\x{01E4}\\x{01E6}\\x{01F4}\\x{1E20}\\x{A7A0}g\\x{011D}\\x{011F}\\x{0121}\\x{0123}\\x{01E5}\\x{01E7}\\x{01F5}\\x{0260}\\x{1D83}\\x{1E21}\\x{A7A1}][\\x{0300}-\\x{036F}]*"
#define H_CLASS "[H\\x{0124}\\x{0126}\\x{021E}\\x{1E22}\\x{1E24}\\x{1E26}\\x{1E28}\\x{1E2A}\\x{2C67}\\x{A7AA}h\\x{0125}\\x{0127}\\x{021F}\\x{0266}\\x{1E23}\\x{1E25}\\x{1E27}\\x{1E29}\\x{1E2B}\\x{1E96}\\x{2C68}\\x{A795}][\\x{0300}-\\x{036F}]*"
#define I_CLASS "[I\\x{00CC}\\x{00CD}\\x{00CE}\\x{00CF}\\x{0128}\\x{012A}\\x{012C}\\x{012E}\\x{0130}\\x{0197}\\x{01CF}\\x{0208}\\x{020A}\\x{1E2C}\\x{1E2E}\\x{1EC8}\\x{1ECA}i\\x{00EC}\\x{00ED}\\x{00EE}\\x{00EF}\\x{0129}\\x{012B}\\x{012D}\\x{012F}\\x{01D0}\\x{0209}\\x{020B}\\x{0268}\\x{1D96}\\x{1E2D}\\x{1E2F}\\x{1EC9}\\x{1ECB}][\\x{0300}-\\x{036F}]*"
#define J_CLASS "[J\\x{0134}\\x{0248}\\x{A7B2}j\\x{0135}\\x{01F0}\\x{0249}\\x{029D}][\\x{0300}-\\x{036F}]*"
#define K_CLASS "[K\\x{0136}\\x{0198}\\x{01E8}\\x{1E30}\\x{1E32}\\x{1E34}\\x{2C69}\\x{A740}\\x{A742}\\x{A744}\\x{A7A2}k\\x{0137}\\x{0199}\\x{01E9}\\x{1D84}\\x{1E31}\\x{1E33}\\x{1E35}\\x{2C6A}\\x{A741}\\x{A743}\\x{A745}\\x{A7A3}][\\x{0300}-\\x{036F}]*"
#define L_CLASS "[L\\x{0139}\\x{013B}\\x{013D}\\x{013F}\\x{0141}\\x{01C8}\\x{023D}\\x{1E36}\\x{1E38}\\x{1E3A}\\x{1E3C}\\x{2C60}\\x{2C62}\\x{A748}\\x{A7AD}l\\x{013A}\\x{013C}\\x{013E}\\x{0140}\\x{0142}\\x{019A}\\x{0234}\\x{026B}\\x{026C}\\x{026D}\\x{1D85}\\x{1E37}\\x{1E39}\\x{1E3B}\\x{1E3D}\\x{2C61}\\x{A749}\\x{A78E}\\x{AB37}\\x{AB38}\\x{AB39}][\\x{0300}-\\x{036F}]*"
#define M_CLASS "[m\\x{0271}\\x{1D6F}\\x{1D86}\\x{1E3F}\\x{1E41}\\x{1E43}\\x{AB3A}M\\x{1E3E}\\x{1E40}\\x{1E42}\\x{2C6E}][\\x{0300}-\\x{036F}]*"
#define N_CLASS "[N\\x{00D1}\\x{0143}\\x{0145}\\x{0147}\\x{019D}\\x{01CB}\\x{01F8}\\x{0220}\\x{1E44}\\x{1E46}\\x{1E48}\\x{1E4A}\\x{A790}\\x{A7A4}n\\x{00F1}\\x{0144}\\x{0146}\\x{0148}\\x{0149}\\x{019E}\\x{01F9}\\x{0235}\\x{0272}\\x{0273}\\x{1D70}\\x{1D87}\\x{1E45}\\x{1E47}\\x{1E49}\\x{1E4B}\\x{A791}\\x{A7A5}\\x{AB3B}][\\x{0300}-\\x{036F}]*"
#define O_CLASS "[O\\x{00D2}\\x{00D3}\\x{00D4}\\x{00D5}\\x{00D6}\\x{00D8}\\x{014C}\\x{014E}\\x{0150}\\x{019F}\\x{01A0}\\x{01D1}\\x{01EA}\\x{01EC}\\x{01FE}\\x{020C}\\x{020E}\\x{022A}\\x{022C}\\x{022E}\\x{0230}\\x{1E4C}\\x{1E4E}\\x{1E50}\\x{1E52}\\x{1ECC}\\x{1ECE}\\x{1ED0}\\x{1ED2}\\x{1ED4}\\x{1ED6}\\x{1ED8}\\x{1EDA}\\x{1EDC}\\x{1EDE}\\x{1EE0}\\x{1EE2}\\x{A74A}\\x{A74C}o\\x{00F2}\\x{00F3}\\x{00F4}\\x{00F5}\\x{00F6}\\x{00F8}\\x{014D}\\x{014F}\\x{0151}\\x{01A1}\\x{01D2}\\x{01EB}\\x{01ED}\\x{01FF}\\x{020D}\\x{020F}\\x{022B}\\x{022D}\\x{022F}\\x{0231}\\x{1E4D}\\x{1E4F}\\x{1E51}\\x{1E53}\\x{1ECD}\\x{1ECF}\\x{1ED1}\\x{1ED3}\\x{1ED5}\\x{1ED7}\\x{1ED9}\\x{1EDB}\\x{1EDD}\\x{1EDF}\\x{1EE1}\\x{1EE3}\\x{2C7A}\\x{A74B}\\x{A74D}][\\x{0300}-\\x{036F}]*"
#define P_CLASS "[P\\x{01A4}\\x{1E54}\\x{1E56}\\x{2C63}\\x{A750}\\x{A752}\\x{A754}p\\x{01A5}\\x{1D71}\\x{1D7D}\\x{1D88}\\x{1E55}\\x{1E57}\\x{A751}\\x{A753}\\x{A755}][\\x{0300}-\\x{036F}]*"
#define Q_CLASS "[q\\x{024B}\\x{02A0}\\x{A757}\\x{A759}Q\\x{A756}\\x{A758}][\\x{0300}-\\x{036F}]*"
#define R_CLASS "[R\\x{0154}\\x{0156}\\x{0158}\\x{0210}\\x{0212}\\x{024C}\\x{1E58}\\x{1E5A}\\x{1E5C}\\x{1E5E}\\x{2C64}\\x{A75A}\\x{A7A6}r\\x{0155}\\x{0157}\\x{0159}\\x{0211}\\x{0213}\\x{024D}\\x{027C}\\x{027D}\\x{027E}\\x{1D72}\\x{1D73}\\x{1D89}\\x{1E59}\\x{1E5B}\\x{1E5D}\\x{1E5F}\\x{A75B}\\x{A7A7}\\x{AB47}\\x{AB49}][\\x{0300}-\\x{036F}]*"
#define S_CLASS "[S\\x{015A}\\x{015C}\\x{015E}\\x{0160}\\x{0218}\\x{1E60}\\x{1E62}\\x{1E64}\\x{1E66}\\x{1E68}\\x{2C7E}\\x{A7A8}s\\x{015B}\\x{015D}\\x{015F}\\x{0161}\\x{0219}\\x{023F}\\x{0282}\\x{1D74}\\x{1D8A}\\x{1E61}\\x{1E63}\\x{1E65}\\x{1E67}\\x{1E69}\\x{A7A9}][\\x{0300}-\\x{036F}]*"
#define T_CLASS "[T\\x{0162}\\x{0164}\\x{0166}\\x{01AC}\\x{01AE}\\x{021A}\\x{023E}\\x{1E6A}\\x{1E6C}\\x{1E6E}\\x{1E70}t\\x{0163}\\x{0165}\\x{0167}\\x{01AB}\\x{01AD}\\x{021B}\\x{0236}\\x{0288}\\x{1D75}\\x{1E6B}\\x{1E6D}\\x{1E6F}\\x{1E71}\\x{1E97}\\x{2C66}][\\x{0300}-\\x{036F}]*"
#define U_CLASS "[U\\x{00D9}\\x{00DA}\\x{00DB}\\x{00DC}\\x{0168}\\x{016A}\\x{016C}\\x{016E}\\x{0170}\\x{0172}\\x{01AF}\\x{01D3}\\x{01D5}\\x{01D7}\\x{01D9}\\x{01DB}\\x{0214}\\x{0216}\\x{0244}\\x{1E72}\\x{1E74}\\x{1E76}\\x{1E78}\\x{1E7A}\\x{1EE4}\\x{1EE6}\\x{1EE8}\\x{1EEA}\\x{1EEC}\\x{1EEE}\\x{1EF0}\\x{A7B8}u\\x{00F9}\\x{00FA}\\x{00FB}\\x{00FC}\\x{0169}\\x{016B}\\x{016D}\\x{016F}\\x{0171}\\x{0173}\\x{01B0}\\x{01D4}\\x{01D6}\\x{01D8}\\x{01DA}\\x{01DC}\\x{0215}\\x{0217}\\x{0289}\\x{1D99}\\x{1E73}\\x{1E75}\\x{1E77}\\x{1E79}\\x{1E7B}\\x{1EE5}\\x{1EE7}\\x{1EE9}\\x{1EEB}\\x{1EED}\\x{1EEF}\\x{1EF1}\\x{A7B9}\\x{AB4E}\\x{AB4F}\\x{AB52}][\\x{0300}-\\x{036F}]*"
#define V_CLASS "[V\\x{01B2}\\x{1E7C}\\x{1E7E}\\x{A75E}v\\x{028B}\\x{1D8C}\\x{1E7D}\\x{1E7F}\\x{2C71}\\x{2C74}\\x{A75F}][\\x{0300}-\\x{036F}]*"
#define W_CLASS "[W\\x{0174}\\x{1E80}\\x{1E82}\\x{1E84}\\x{1E86}\\x{1E88}\\x{2C72}w\\x{0175}\\x{1E81}\\x{1E83}\\x{1E85}\\x{1E87}\\x{1E89}\\x{1E98}\\x{2C73}][\\x{0300}-\\x{036F}]*"
#define X_CLASS "[x\\x{1D8D}\\x{1E8B}\\x{1E8D}\\x{AB56}\\x{AB57}\\x{AB58}\\x{AB59}X\\x{1E8A}\\x{1E8C}][\\x{0300}-\\x{036F}]*"
#define Y_CLASS "[Y\\x{00DD}\\x{0176}\\x{0178}\\x{01B3}\\x{0232}\\x{024E}\\x{1E8E}\\x{1EF2}\\x{1EF4}\\x{1EF6}\\x{1EF8}\\x{1EFE}y\\x{00FD}\\x{00FF}\\x{0177}\\x{01B4}\\x{0233}\\x{024F}\\x{1E8F}\\x{1E99}\\x{1EF3}\\x{1EF5}\\x{1EF7}\\x{1EF9}\\x{1EFF}\\x{AB5A}][\\x{0300}-\\x{036F}]*"
#define Z_CLASS "[Z\\x{0179}\\x{017B}\\x{017D}\\x{01B5}\\x{0224}\\x{1E90}\\x{1E92}\\x{1E94}\\x{2C6B}\\x{2C7F}z\\x{017A}\\x{017C}\\x{017E}\\x{01B6}\\x{0225}\\x{0240}\\x{0290}\\x{0291}\\x{1D76}\\x{1D8E}\\x{1E91}\\x{1E93}\\x{1E95}\\x{2C6C}][\\x{0300}-\\x{036F}]*"

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

#define nregexes 28 // Latin alphabet plus the two wildcards.

static const character_class_t CLASSES[nregexes] = {
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

#define nnormalizedletters 26 // Just the Latin alphabet.

static const character_class_t NORMALIZED[nnormalizedletters] = {
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

static inline void merge_array_insert(merge_element_t* merge_array, int* delta, int offset, int size, const character_class_t* class) {
    merge_element_t* element = &merge_array[offset];
    element->size = size;
    element->class = class;

    *delta += (class->replacement_length - size);
}

static void like_pattern2pcre_pattern(
    char** result_cursor,
    const char* pat,
    const character_class_t* classes, int nclasses,
    merge_element_t* merge_array, int pat_length) {
    (void) pat;
    int delta = 0; // Counts the increase in bytes of the transformed pattern w.r.t. the original pattern
    const char* err = NULL;
    int erroffset;

    pcre* compiled_patterns[nclasses];

    int ovector[ovector_length];

    for (int i = 0; i < nclasses; i ++) {
        const character_class_t* class = &classes[i];

        // TODO Do all of this stuff in a mall prelude
        int options = PCRE_UTF8 | PCRE_MULTILINE | PCRE_DOTALL;
        compiled_patterns[i] = pcre_compile(class->match, options, &err, &erroffset, NULL);

        if (err) {
            printf("%s\n", err);
        }

        int pos = -1;
        int begin = 0;
        int end   = 0;

        while ( (pos = pcre_exec(compiled_patterns[i], NULL, pat, pat_length, end, 0, ovector, ovector_length)) > PCRE_ERROR_NOMATCH ) {
            begin = ovector[0];
            end   = ovector[1];
            int size = end - begin;

            merge_array_insert(merge_array, &delta, begin, size, class);
        }
        // TODO handle pos < -1 aka errors
    }

    for (int i = 0; i < pat_length;) {
        merge_element_t* merge_element = &merge_array[i];

        if (merge_element->size) {

            strncpy(*result_cursor, merge_element->class->replacement, merge_element->class->replacement_length);
            *result_cursor += merge_element->class->replacement_length;
            i+=merge_element->size;

            continue;
        }

        *(*result_cursor)++ = *(pat + i++); // Just copy the current byte
    }

    **result_cursor = 0; // Finalize transformed_pat with null terminator
}

char *UDFpattern2re(char ** result, const char **input) {
    if (GDK_STRNIL(*input)) {

    // TODO: check for allocation errors.
        *result = GDKmalloc(sizeof(str_nil));
        strcpy (*result, str_nil);

	    return MAL_SUCCEED;
    }

    int pat_length =  strlen(*input);

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKzalloc(pat_length * sizeof(merge_element_t));

    int buffer_size = strlen(O_CLASS) * (pat_length + 2) +1;

    // TODO: check for allocation errors.
    *result = GDKzalloc(buffer_size);

    char* cursor = *result;
    *cursor++ = '^'; // match start of string.

    // TODO error handling
    like_pattern2pcre_pattern(&cursor, *input, CLASSES, nregexes, merge_array, pat_length);

    *cursor++ = '$'; // Overwrite null terminator and match end of string.

    *cursor = 0; // Finalize transformed_pat with null terminator.

    GDKfree(merge_array);

	return MAL_SUCCEED;
}

char *UDFpattern2normalized(char ** result, const char **input) {
    if (GDK_STRNIL(*input)) {

    // TODO: check for allocation errors.
        *result = GDKmalloc(sizeof(str_nil));
        strcpy (*result, str_nil);

	    return MAL_SUCCEED;
    }

    int pat_length =  strlen(*input);

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKzalloc(pat_length * sizeof(merge_element_t));

    int buffer_size = pat_length + 1;

    // TODO: check for allocation errors.
    *result = GDKzalloc(buffer_size);

    char* cursor = *result;

    // TODO error handling
    like_pattern2pcre_pattern(&cursor, *input, NORMALIZED, nnormalizedletters, merge_array, pat_length);

    GDKfree(merge_array);

	return MAL_SUCCEED;
}

char *
UDFBATpattern2normalized(bat *result, const bat *input) {
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

    int pat_length = 4;

    // TODO: check for allocation errors.
    merge_element_t* merge_array = (merge_element_t*) GDKmalloc(pat_length * sizeof(merge_element_t));

    // TODO: check for allocation errors.
    char* result_val = GDKmalloc(pat_length + 1);

	bi = bat_iterator(input_bat);
	BATloop(input_bat, p, q) {
		const char *source = (const char *) BUNtail(bi, p);

		if (GDK_STRNIL(source)) {
            if (BUNappend(result_bat, str_nil, false) != GDK_SUCCEED) {
                // TODO error handling
            }

            continue;
		}

        const int source_length = strlen(source);

		if (source_length >= pat_length) {

            pat_length = source_length;

            merge_array = GDKrealloc(merge_array, pat_length * sizeof(merge_element_t));

            result_val = GDKrealloc(result_val, pat_length + 1);
		}

        memset(merge_array, 0, pat_length * sizeof(merge_element_t));

        char* cursor = result_val;

        // TODO error handling
        like_pattern2pcre_pattern(&cursor, source, NORMALIZED, nnormalizedletters, merge_array, source_length);

		if (BUNappend(result_bat, result_val, false) != GDK_SUCCEED) {
			// TODO error handling
		}
	}

	GDKfree(merge_array);
    GDKfree(result_val);
	BBPunfix(input_bat->batCacheid);

	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	return MAL_SUCCEED;
}
