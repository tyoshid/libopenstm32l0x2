#include <stdio.h>

#include "font_space.c"
#include "font_exclamation.c"
#include "font_double_quotes.c"
#include "font_number.c"
#include "font_dollar.c"
#include "font_percent.c"
#include "font_ampersand.c"
#include "font_single_quote.c"
#include "font_opening_parenthesis.c"
#include "font_closing_parenthesis.c"
#include "font_asterisk.c"
#include "font_plus.c"
#include "font_comma.c"
#include "font_minus.c"
#include "font_period.c"
#include "font_slash.c"

#include "font_0.c"
#include "font_1.c"
#include "font_2.c"
#include "font_3.c"
#include "font_4.c"
#include "font_5.c"
#include "font_6.c"
#include "font_7.c"
#include "font_8.c"
#include "font_9.c"
#include "font_colon.c"
#include "font_semicolon.c"
#include "font_less_than.c"
#include "font_equal.c"
#include "font_greater_than.c"
#include "font_question.c"

#include "font_at.c"
#include "font_A.c"
#include "font_B.c"
#include "font_C.c"
#include "font_D.c"
#include "font_E.c"
#include "font_F.c"
#include "font_G.c"
#include "font_H.c"
#include "font_I.c"
#include "font_J.c"
#include "font_K.c"
#include "font_L.c"
#include "font_M.c"
#include "font_N.c"
#include "font_O.c"

#include "font_P.c"
#include "font_Q.c"
#include "font_R.c"
#include "font_S.c"
#include "font_T.c"
#include "font_U.c"
#include "font_V.c"
#include "font_W.c"
#include "font_X.c"
#include "font_Y.c"
#include "font_Z.c"
#include "font_opening_bracket.c"
#include "font_backslash.c"
#include "font_closing_bracket.c"
#include "font_caret.c"
#include "font_underscore.c"

#include "font_grave_accent.c"
#include "font_a.c"
#include "font_b.c"
#include "font_c.c"
#include "font_d.c"
#include "font_e.c"
#include "font_f.c"
#include "font_g.c"
#include "font_h.c"
#include "font_i.c"
#include "font_j.c"
#include "font_k.c"
#include "font_l.c"
#include "font_m.c"
#include "font_n.c"
#include "font_o.c"

#include "font_p.c"
#include "font_q.c"
#include "font_r.c"
#include "font_s.c"
#include "font_t.c"
#include "font_u.c"
#include "font_v.c"
#include "font_w.c"
#include "font_x.c"
#include "font_y.c"
#include "font_z.c"
#include "font_opening_brace.c"
#include "font_vertical_bar.c"
#include "font_closing_brace.c"
#include "font_tilde.c"

void print_data(const unsigned char *p, int width, int height)
{
	int page;
	int column;
	int bit;
	unsigned char d;

	for (page = 0; page < height / 8; page++) {
		for (column = 0; column < width; column++) {
			if (column % 8 == 0)
				printf("\t");
			d = 0;
			for (bit = 0; bit < 8; bit++)
				d |= (*(p + page * 8 * width * 3 +
					bit * width * 3 + column * 3)) ? 0 :
					(1 << bit);
			printf("0x%02x,%c", d,
			       (column % 8 == 7 || column == width - 1) ?
			       '\n' : ' ');
		}
	}
}

int main()
{
	printf("const char font[] = {\n");
	/* space */
	print_data(font_space.pixel_data, 6, 8);
	/* exclamation point */
	print_data(font_exclamation.pixel_data, 6, 8);
	/* double quotes */
	print_data(font_double_quotes.pixel_data, 6, 8);
	/* number sign */
	print_data(font_number.pixel_data, 6, 8);
	/* dollar sign */
	print_data(font_dollar.pixel_data, 6, 8);
	/* percent sign */
	print_data(font_percent.pixel_data, 6, 8);
	/* ampersand */
	print_data(font_ampersand.pixel_data, 6, 8);
	/* single quote */
	print_data(font_single_quote.pixel_data, 6, 8);
	/* opening parenthesis */
	print_data(font_opening_parenthesis.pixel_data, 6, 8);
	/* closing parenthesis */
	print_data(font_closing_parenthesis.pixel_data, 6, 8);
	/* asterisk */
	print_data(font_asterisk.pixel_data, 6, 8);
	/* plus sign */
	print_data(font_plus.pixel_data, 6, 8);
	/* comma */
	print_data(font_comma.pixel_data, 6, 8);
	/* minus sign */
	print_data(font_minus.pixel_data, 6, 8);
	/* period */
	print_data(font_period.pixel_data, 6, 8);
	/* slash */
	print_data(font_slash.pixel_data, 6, 8);

	/* 0 */
	print_data(font_0.pixel_data, 6, 8);
	/* 1 */
	print_data(font_1.pixel_data, 6, 8);
	/* 2 */
	print_data(font_2.pixel_data, 6, 8);
	/* 3 */
	print_data(font_3.pixel_data, 6, 8);
	/* 4 */
	print_data(font_4.pixel_data, 6, 8);
	/* 5 */
	print_data(font_5.pixel_data, 6, 8);
	/* 6 */
	print_data(font_6.pixel_data, 6, 8);
	/* 7 */
	print_data(font_7.pixel_data, 6, 8);
	/* 8 */
	print_data(font_8.pixel_data, 6, 8);
	/* 9 */
	print_data(font_9.pixel_data, 6, 8);
	/* colon */
	print_data(font_colon.pixel_data, 6, 8);
	/* semicolon */
	print_data(font_semicolon.pixel_data, 6, 8);
	/* less than sign */
	print_data(font_less_than.pixel_data, 6, 8);
	/* equal sign */
	print_data(font_equal.pixel_data, 6, 8);
	/* greater than sign */
	print_data(font_greater_than.pixel_data, 6, 8);
	/* question mark */
	print_data(font_question.pixel_data, 6, 8);

	/* at symbol */
	print_data(font_at.pixel_data, 6, 8);
	/* A */
	print_data(font_A.pixel_data, 6, 8);
	/* B */
	print_data(font_B.pixel_data, 6, 8);
	/* C */
	print_data(font_C.pixel_data, 6, 8);
	/* D */
	print_data(font_D.pixel_data, 6, 8);
	/* E */
	print_data(font_E.pixel_data, 6, 8);
	/* F */
	print_data(font_F.pixel_data, 6, 8);
	/* G */
	print_data(font_G.pixel_data, 6, 8);
	/* H */
	print_data(font_H.pixel_data, 6, 8);
	/* I */
	print_data(font_I.pixel_data, 6, 8);
	/* J */
	print_data(font_J.pixel_data, 6, 8);
	/* K */
	print_data(font_K.pixel_data, 6, 8);
	/* L */
	print_data(font_L.pixel_data, 6, 8);
	/* M */
	print_data(font_M.pixel_data, 6, 8);
	/* N */
	print_data(font_N.pixel_data, 6, 8);
	/* O */
	print_data(font_O.pixel_data, 6, 8);

	/* P */
	print_data(font_P.pixel_data, 6, 8);
	/* Q */
	print_data(font_Q.pixel_data, 6, 8);
	/* R */
	print_data(font_R.pixel_data, 6, 8);
	/* S */
	print_data(font_S.pixel_data, 6, 8);
	/* T */
	print_data(font_T.pixel_data, 6, 8);
	/* U */
	print_data(font_U.pixel_data, 6, 8);
	/* V */
	print_data(font_V.pixel_data, 6, 8);
	/* W */
	print_data(font_W.pixel_data, 6, 8);
	/* X */
	print_data(font_X.pixel_data, 6, 8);
	/* Y */
	print_data(font_Y.pixel_data, 6, 8);
	/* Z */
	print_data(font_Z.pixel_data, 6, 8);
	/* opening bracket */
	print_data(font_opening_bracket.pixel_data, 6, 8);
	/* backslash */
	print_data(font_backslash.pixel_data, 6, 8);
	/* closing bracket */
	print_data(font_closing_bracket.pixel_data, 6, 8);
	/* caret */
	print_data(font_caret.pixel_data, 6, 8);
	/* undercore */
	print_data(font_underscore.pixel_data, 6, 8);

	/* grave accent */
	print_data(font_grave_accent.pixel_data, 6, 8);
	/* a */
	print_data(font_a.pixel_data, 6, 8);
	/* b */
	print_data(font_b.pixel_data, 6, 8);
	/* c */
	print_data(font_c.pixel_data, 6, 8);
	/* d */
	print_data(font_d.pixel_data, 6, 8);
	/* e */
	print_data(font_e.pixel_data, 6, 8);
	/* f */
	print_data(font_f.pixel_data, 6, 8);
	/* g */
	print_data(font_g.pixel_data, 6, 8);
	/* h */
	print_data(font_h.pixel_data, 6, 8);
	/* i */
	print_data(font_i.pixel_data, 6, 8);
	/* j */
	print_data(font_j.pixel_data, 6, 8);
	/* k */
	print_data(font_k.pixel_data, 6, 8);
	/* l */
	print_data(font_l.pixel_data, 6, 8);
	/* m */
	print_data(font_m.pixel_data, 6, 8);
	/* n */
	print_data(font_n.pixel_data, 6, 8);
	/* o */
	print_data(font_o.pixel_data, 6, 8);

	/* p */
	print_data(font_p.pixel_data, 6, 8);
	/* q */
	print_data(font_q.pixel_data, 6, 8);
	/* r */
	print_data(font_r.pixel_data, 6, 8);
	/* s */
	print_data(font_s.pixel_data, 6, 8);
	/* t */
	print_data(font_t.pixel_data, 6, 8);
	/* u */
	print_data(font_u.pixel_data, 6, 8);
	/* v */
	print_data(font_v.pixel_data, 6, 8);
	/* w */
	print_data(font_w.pixel_data, 6, 8);
	/* x */
	print_data(font_x.pixel_data, 6, 8);
	/* y */
	print_data(font_y.pixel_data, 6, 8);
	/* z */
	print_data(font_z.pixel_data, 6, 8);
	/* opening brace */
	print_data(font_opening_brace.pixel_data, 6, 8);
	/* vertical bar */
	print_data(font_vertical_bar.pixel_data, 6, 8);
	/* closing brace */
	print_data(font_closing_brace.pixel_data, 6, 8);
	/* tilde */
	print_data(font_tilde.pixel_data, 6, 8);

	printf("};\n");

	return 0;
}
