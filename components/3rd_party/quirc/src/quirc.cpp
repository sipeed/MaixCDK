#include "quirc.hpp"
#include "quirc.h"
#include "stdio.h"
#include "cstring"
void quirc_scan_qrcode_in_gray(uint8_t *gray, int width, int height, quirc_qrcode_result_t *result)
{
    quirc *q = quirc_new();
    if (!q) {
        printf("quirc_new failed\r\n");
        return;
    }

	if (quirc_resize(q, width, height) < 0) {
		perror("couldn't allocate QR buffer");
	}

    uint8_t *quirc_src = quirc_begin(q, NULL, NULL);
    memcpy(quirc_src, gray, width * height);
    quirc_end(q);

    for (int i = 0; i < quirc_count(q); i++) {
        quirc_code code;
        struct quirc_data data;
		quirc_decode_error_t err;
        quirc_extract(q, i, &code);

		// for (int j = 0; j < 4; j++) {
		// 	struct quirc_point *a = &code.corners[j];
		// 	struct quirc_point *b = &code.corners[(j + 1) % 4];
        //     printf("corners %d %d %d %d\r\n", a->x, a->y, b->x, b->y);
		// }

        err = quirc_decode(&code, &data);
        if (result) {
            strncpy(result->data, (char *)data.payload, sizeof(result->data));
        }
    }

    quirc_destroy(q);
}
