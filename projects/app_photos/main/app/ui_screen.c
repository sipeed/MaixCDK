#include "lvgl.h"
#include "stdio.h"
#include "ui_screen.h"
#include "ui_event_handler.h"

typedef struct Node Node;

typedef struct {
    char dir_name[20];
    Node *list;
} ui_directory_t;

typedef struct {
    union {
        ui_photo_t photo;
        ui_directory_t dir;
    };
} NodeItem;

typedef struct Node {
    NodeItem *data;
    struct Node* next;
} Node;

typedef struct {
    int max_w;
    int max_h;
    Node *dirs;
    int photo_list_screen_scroll_y;
    int need_bulk_delete;
    int view_flag;    // flag: 0, view in small photo screen; 1, view in big photo screen
    ui_big_photo_info_t big_image_info;
    Node *bulk_delete_iter_dir;
    Node *bulk_delete_iter_photo;
} priv_t;

static priv_t priv;

lv_obj_t *g_base_screen;
lv_obj_t *g_lower_screen;
lv_obj_t *g_lower_view1;
lv_obj_t *g_lower_view2;
lv_obj_t *g_upper_screen;
lv_obj_t *g_right_screen;
lv_obj_t *g_right_info;
lv_obj_t *g_big_photo_screen;
lv_obj_t *g_small_photo_screen;
lv_obj_t *g_switch_left;
lv_obj_t *g_switch_right;
lv_obj_t *g_video_screen;
lv_obj_t *g_video_bar_label;
lv_obj_t *g_video_bar;
lv_obj_t *g_video_pause_img;

LV_IMG_DECLARE(img_return);
LV_IMG_DECLARE(img_delete);
LV_IMG_DECLARE(img_cancel);
LV_IMG_DECLARE(img_option);
LV_IMG_DECLARE(img_right);
LV_IMG_DECLARE(img_left);
LV_IMG_DECLARE(img_video_play);

extern void event_touch_small_image_cb(lv_event_t * e);
extern void event_touch_big_image_cb(lv_event_t * e);
extern void event_touch_exit_cb(lv_event_t * e);
extern void event_touch_bulk_delete_cb(lv_event_t * e);
extern void event_touch_bulk_delete_cancel_cb(lv_event_t * e);
extern void event_touch_delete_big_photo_cb(lv_event_t * e);
extern void event_touch_show_big_photo_info_cb(lv_event_t * e);
extern void event_touch_show_left_big_photo_cb(lv_event_t * e);
extern void event_touch_show_right_big_photo_cb(lv_event_t * e);

static void priv_init(void)
{
    priv.max_w = 552;
    priv.max_h = 368;
}

static Node* createNode(NodeItem *data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to create node");
        return NULL;
    }

    NodeItem* newItem = NULL;
    if (data) {
        newItem = (NodeItem*)malloc(sizeof(NodeItem));
        if (newItem == NULL) {
            perror("Failed to create node");
            return NULL;
        }

        memcpy(newItem, data, sizeof(NodeItem));
    }

    newNode->data = newItem;
    newNode->next = NULL;
    return newNode;
}
#if 0
static void destroyDirNode(Node **head) {
    Node* next = *head;
    Node *temp;
    while (next != NULL) {
        temp = next->next;
        if (temp->data) {
            free(temp->data);
        }
        free(temp);
    }
}
#endif
static void destroyPicNode(Node **head) {
    Node* next = *head;
    Node *temp = next;
    while (next != NULL) {
        temp = next;
        next = next->next;
        if (temp->data) {
            if (temp->data->photo.img_dsc) {
                free(temp->data->photo.img_dsc);
            }

            if (temp->data->photo.path) {
                free(temp->data->photo.path);
            }
            free(temp->data);
        }
        free(temp);

    }
}

static void appendNode(Node** head, NodeItem *data) {
    Node* newNode = createNode(data);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

static Node *selectDirNode(Node *head, char *dir_name) {
    Node* temp = head;
    while (temp != NULL) {
        if (temp->data) {
            if (!strcmp(temp->data->dir.dir_name, dir_name)) {
                return temp;
            }
        } else {
            printf("unknown error when select directory!\r\n");
            return NULL;
        }
        temp = temp->next;
    }

    return NULL;
}
#if 0
static Node *selectPicNode(Node *head, char *path) {
    Node* temp = head;
    while (temp != NULL) {
        if (!strcmp(temp->data->photo.path, path)) {
            return temp;
        }
        temp = temp->next;
    }

    return NULL;
}
#endif
static void deletePicNode(Node** head, char* path) {
    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && !strcmp(path, temp->data->photo.path)) {
        *head = temp->next;
        if (temp->data) {
            if (temp->data->photo.img_dsc) {
                if (temp->data->photo.img_dsc->data) {
                    free((uint8_t *)temp->data->photo.img_dsc->data);
                    temp->data->photo.img_dsc->data = NULL;
                }
                free(temp->data->photo.img_dsc);
            }

            if (temp->data->photo.path) {
                free(temp->data->photo.path);
            }
            free(temp->data);
        }
        free(temp);
        return;
    }

    while (temp != NULL && strcmp(path, temp->data->photo.path)) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    if (temp->data) {
        if (temp->data->photo.img_dsc) {
            free(temp->data->photo.img_dsc);
        }

        if (temp->data->photo.path) {
            free(temp->data->photo.path);
        }
        free(temp->data);
    }
    free(temp);
}

static void deleteDirNode(Node** head, char* dir_name) {
    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && !strcmp(dir_name, temp->data->dir.dir_name)) {
        *head = temp->next;
        if (temp->data->dir.list) {
            destroyPicNode(&temp->data->dir.list);
        }

        free(temp);
        return;
    }

    while (temp != NULL && strcmp(dir_name, temp->data->dir.dir_name)) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    prev->next = temp->next;
    if (temp->data->dir.list) {
        destroyPicNode(&temp->data->dir.list);
    }
    free(temp);
}

static void _ui_base_screen_init(void)
{
    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(scr, 0, 0);

    g_base_screen = scr;
}

static void _ui_upper_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(12));
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_size(sub_obj, lv_pct(12), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(sub_obj, event_touch_exit_cb, LV_EVENT_PRESSED, NULL);

        lv_obj_t *img = lv_image_create(sub_obj);
        lv_image_set_src(img, &img_return);
        lv_obj_center(img);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(12), 0);
        lv_obj_set_size(sub_obj, lv_pct(76) + 1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "Photos");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(88), 0);
        lv_obj_set_size(sub_obj, lv_pct(12), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
    }

    g_upper_screen = obj;
}

// flag: 0, view in small image screen; 1, view in big image screen
static void _ui_lower_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_layer_sys());
    lv_obj_set_align(obj, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(15));
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    {
        lv_obj_t *obj2 = lv_obj_create(obj);
        lv_obj_set_align(obj2, LV_ALIGN_BOTTOM_MID);
        lv_obj_set_size(obj2, lv_pct(100), lv_pct(100));
        lv_obj_set_style_radius(obj2, 0, 0);
        lv_obj_remove_flag(obj2, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(obj2, 0, 0);
        lv_obj_set_style_border_side(obj2, LV_BORDER_SIDE_NONE, 0);

        {
            lv_obj_t *sub_obj = lv_obj_create(obj2);
            lv_obj_set_size(sub_obj, lv_pct(50), lv_pct(100));
            lv_obj_set_style_radius(sub_obj, 0, 0);
            lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_style_outline_pad(sub_obj, 0, 0);
            lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(sub_obj, event_touch_bulk_delete_cb, LV_EVENT_CLICKED, NULL);

            lv_obj_t *img = lv_image_create(sub_obj);
            lv_image_set_src(img, &img_delete);
            lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

            lv_obj_t *label = lv_label_create(sub_obj);
            lv_label_set_text(label, "delete");
            lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        {
            lv_obj_t *sub_obj = lv_obj_create(obj2);
            lv_obj_set_pos(sub_obj, lv_pct(50), 0);
            lv_obj_set_size(sub_obj, lv_pct(50), lv_pct(100));
            lv_obj_set_style_radius(sub_obj, 0, 0);
            lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_style_outline_pad(sub_obj, 0, 0);
            lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(sub_obj, event_touch_bulk_delete_cancel_cb, LV_EVENT_CLICKED, NULL);

            lv_obj_t *img = lv_image_create(sub_obj);
            lv_image_set_src(img, &img_cancel);
            lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

            lv_obj_t *label = lv_label_create(sub_obj);
            lv_label_set_text(label, "cancel");
            lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        g_lower_view1 = obj2;
    }

    {
        lv_obj_t *obj2 = lv_obj_create(obj);
        lv_obj_set_align(obj2, LV_ALIGN_BOTTOM_MID);
        lv_obj_set_size(obj2, lv_pct(100), lv_pct(100));
        lv_obj_set_style_radius(obj2, 0, 0);
        lv_obj_remove_flag(obj2, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(obj2, 0, 0);
        lv_obj_set_style_border_side(obj2, LV_BORDER_SIDE_NONE, 0);

        {
            lv_obj_t *sub_obj = lv_obj_create(obj2);
            lv_obj_set_size(sub_obj, lv_pct(50), lv_pct(100));
            lv_obj_set_style_radius(sub_obj, 0, 0);
            lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_style_outline_pad(sub_obj, 0, 0);
            lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(sub_obj, event_touch_show_big_photo_info_cb, LV_EVENT_CLICKED, NULL);

            lv_obj_t *img = lv_image_create(sub_obj);
            lv_image_set_src(img, &img_option);
            lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

            lv_obj_t *label = lv_label_create(sub_obj);
            lv_label_set_text(label, "info");
            lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        {
            lv_obj_t *sub_obj = lv_obj_create(obj2);
            lv_obj_set_pos(sub_obj, lv_pct(50), 0);
            lv_obj_set_size(sub_obj, lv_pct(50), lv_pct(100));
            lv_obj_set_style_radius(sub_obj, 0, 0);
            lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_style_outline_pad(sub_obj, 0, 0);
            lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_event_cb(sub_obj, event_touch_delete_big_photo_cb, LV_EVENT_CLICKED, NULL);

            lv_obj_t *img = lv_image_create(sub_obj);
            lv_image_set_src(img, &img_delete);
            lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

            lv_obj_t *label = lv_label_create(sub_obj);
            lv_label_set_text(label, "delete");
            lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        g_lower_view2 = obj2;
        lv_obj_add_flag(g_lower_view2, LV_OBJ_FLAG_HIDDEN);
    }


    g_lower_screen = obj;
}

static void _ui_right_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_layer_sys());
    lv_obj_set_align(obj, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_pos(obj, 0, lv_pct(12));
    lv_obj_set_size(obj, lv_pct(20), lv_pct(73) + 1);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    g_right_screen = obj;
}

static void _ui_right_screen_info_init(void)
{
    lv_obj_t *obj = g_right_screen;
    lv_obj_t *obj2 = lv_obj_create(obj);
    lv_obj_set_size(obj2, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(obj2, 0, 0);
    lv_obj_remove_flag(obj2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj2, 0, 0);
    lv_obj_set_style_bg_color(obj2, lv_color_hex(0x0), 0);
    lv_obj_set_style_border_side(obj2, LV_BORDER_SIDE_NONE, 0);

    int width = 0, height = 0;
    int file_size = 0;
    width = priv.big_image_info.src_width;
    height = priv.big_image_info.src_height;
    file_size = priv.big_image_info.size;
    char *type = priv.big_image_info.type;

    char str[1024];
    {
        lv_obj_t *label = lv_label_create(obj2);
        lv_obj_set_pos(label, lv_pct(5), lv_pct(10));
        snprintf(str, sizeof(str), "width: %2d", width);
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label, lv_color_hex(0x6c6c6c), 0);
    }

    {
        lv_obj_t *label = lv_label_create(obj2);
        lv_obj_set_pos(label, lv_pct(5), lv_pct(25));
        snprintf(str, sizeof(str), "height: %2d", height);
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label, lv_color_hex(0x6c6c6c), 0);
    }

    {
        lv_obj_t *label = lv_label_create(obj2);
        lv_obj_set_pos(label, lv_pct(5), lv_pct(40));
        snprintf(str, sizeof(str), "filetype: %s", type);
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label, lv_color_hex(0x6c6c6c), 0);
    }

    {
        lv_obj_t *label = lv_label_create(obj2);
        lv_obj_set_pos(label, lv_pct(5), lv_pct(55));
        if (file_size > 1024 * 1024) {
            file_size /= 1024 * 1024;
            snprintf(str, sizeof(str), "filesize: %dmb", file_size);
        } else if (file_size > 1024) {
            file_size /= 1024;
            snprintf(str, sizeof(str), "filesize: %dkb", file_size);
        } else {
            snprintf(str, sizeof(str), "filesize: %d", file_size);
        }
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label, lv_color_hex(0x6c6c6c), 0);
    }

    {
        lv_obj_t *label = lv_label_create(obj2);
        lv_obj_set_pos(label, lv_pct(5), lv_pct(70));
        lv_obj_set_width(label, lv_pct(100));
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
        char absolute_path[1000];
        if (realpath(priv.big_image_info.path, absolute_path) != NULL) {
            snprintf(str, sizeof(str), "path: %s", absolute_path);
        } else {
            snprintf(str, sizeof(str), "path: none");
        }
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label, lv_color_hex(0x6c6c6c), 0);
    }

    g_right_info = obj2;
}

static void _ui_right_screen_info_deinit(void)
{
    if (g_right_info) {
        lv_obj_delete(g_right_info);
        g_right_info = NULL;
    }
}

void ui_right_screen_update(void)
{
    _ui_right_screen_info_deinit();
    _ui_right_screen_info_init();
}

static void _ui_left_right_init(void)
{
    lv_obj_t *scr = lv_layer_sys();
    lv_obj_t *left = lv_image_create(scr);
    lv_obj_set_align(left, LV_ALIGN_LEFT_MID);
    lv_obj_set_pos(left, lv_pct(0), lv_pct(0));
    lv_obj_add_flag(left, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(left, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_pad_ver(left, 50, 0);
    lv_obj_set_style_pad_right(left, 20, 0);
    lv_obj_add_event_cb(left, event_touch_show_left_big_photo_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_image_set_src(left, &img_left);

    lv_obj_t *right = lv_image_create(scr);
    lv_obj_set_align(right, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(right, lv_pct(0), lv_pct(0));
    lv_obj_add_flag(right, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_pad_ver(right, 50, 0);
    lv_obj_set_style_pad_left(right, 20, 0);
    lv_obj_add_event_cb(right, event_touch_show_right_big_photo_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_image_set_src(right, &img_right);

    g_switch_right = right;
    g_switch_left = left;
}

static void _ui_big_photo_screen_init(void)
{
    lv_obj_t *scr = lv_obj_create(g_base_screen);
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_user_data(scr, &priv.big_image_info);

    lv_obj_t *img = lv_image_create(scr);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, event_touch_big_image_cb, LV_EVENT_SHORT_CLICKED, &priv.big_image_info);

    if (priv.big_image_info.dsc.data) {
        lv_image_set_src(img, &priv.big_image_info.dsc);
    }

    g_big_photo_screen = scr;
}

static void _ui_big_photo_screen_deinit(void)
{
    if (g_big_photo_screen) {
        lv_obj_delete(g_big_photo_screen);
        g_big_photo_screen = NULL;
    }
}

// flag: 0, view in small photo screen; 1, view in big photo screen; 2, view in big photo and don't change upper and lower screen
// 4. view video screen
void ui_set_view_flag(int value)
{
    priv.view_flag = value;
    printf("set view flag:%d\r\n", value);
    switch (priv.view_flag) {
    case 0: // 0, view in small photo screen
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_100, 0);
        }

        if (g_video_screen) {
            lv_obj_add_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) lv_obj_add_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_big_photo_screen) {
            lv_obj_add_flag(g_big_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_small_photo_screen){
            lv_obj_remove_flag(g_small_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_lower_screen) {
            lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_lower_view1) {
                lv_obj_remove_flag(g_lower_view1, LV_OBJ_FLAG_HIDDEN);
            }
            if (g_lower_view2) {
                lv_obj_add_flag(g_lower_view2, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (g_upper_screen) {
            lv_obj_remove_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_right_screen) {
            lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_left) {
            lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_right) {
            lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
        }
    }
        break;
    case 1: // 1, view in big photo screen
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_100, 0);
        }

        if (g_video_screen) {
            lv_obj_add_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) lv_obj_add_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_big_photo_screen) {
            lv_obj_remove_flag(g_big_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_small_photo_screen){
            lv_obj_add_flag(g_small_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_lower_screen) {
            lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_lower_view1) {
                lv_obj_add_flag(g_lower_view1, LV_OBJ_FLAG_HIDDEN);
            }
            if (g_lower_view2) {
                lv_obj_remove_flag(g_lower_view2, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (g_upper_screen) {
            lv_obj_add_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_right_screen) {
            lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_left) {
            lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_right) {
            lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    }
    case 2: // 2, view in big photo screen
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_100, 0);
        }

        if (g_video_screen) {
            lv_obj_add_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) lv_obj_add_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_big_photo_screen) {
            lv_obj_remove_flag(g_big_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_small_photo_screen){
            lv_obj_add_flag(g_small_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_left) {
            if (g_upper_screen) {
                if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_remove_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }

        if (g_switch_right) {
            if (g_upper_screen) {
                if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_remove_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
        break;
    }
    case 3:
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_100, 0);
        }

        if (g_video_screen) {
            lv_obj_add_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) lv_obj_add_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_upper_screen && g_lower_screen) {
            if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN) && lv_obj_has_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_remove_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            } else if (!lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN) && lv_obj_has_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_remove_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (g_switch_left) {
            if (g_upper_screen) {
                if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_remove_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }

        if (g_switch_right) {
            if (g_upper_screen) {
                if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_remove_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
        break;
    }
    case 4:
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_0, 0);
        }

        if (g_video_screen) {
            lv_obj_remove_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) lv_obj_add_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_big_photo_screen) {
            lv_obj_add_flag(g_big_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_small_photo_screen){
            lv_obj_add_flag(g_small_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_lower_screen) {
            lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_lower_view1) {
                lv_obj_add_flag(g_lower_view1, LV_OBJ_FLAG_HIDDEN);
            }
            if (g_lower_view2) {
                lv_obj_remove_flag(g_lower_view2, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (g_upper_screen) {
            lv_obj_add_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_right_screen) {
            lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_left) {
            lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_right) {
            lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    }
    case 5:
    {
        if (g_base_screen) {
            lv_obj_set_style_opa(g_base_screen, LV_OPA_0, 0);
        }

        if (g_video_screen) {
            lv_obj_remove_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);
            if (g_video_pause_img) {
                lv_obj_remove_flag(g_video_pause_img, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (g_big_photo_screen) {
            lv_obj_add_flag(g_big_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_small_photo_screen){
            lv_obj_add_flag(g_small_photo_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_upper_screen) {
            lv_obj_remove_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_right_screen) {
            lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_left) {
            lv_obj_remove_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_switch_right) {
            lv_obj_remove_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    }
    default:break;
    }
}

int ui_get_view_flag(void)
{
    return priv.view_flag;
}

void ui_update_big_photo(ui_big_photo_info_t *info)
{
    lv_image_dsc_t *src = (lv_image_dsc_t *)&priv.big_image_info.dsc;
    if (!src->data) {
        src->data = (uint8_t *)malloc(info->dsc.data_size);
        if (!src->data) {
            perror("upload big photo failed");
            return;
        }
    }

    if (priv.big_image_info.path) {
        free(priv.big_image_info.path);
    }
    priv.big_image_info.path = (char *)malloc(strlen(info->path) + 1);
    if (!priv.big_image_info.path) {
        perror("upload big photo failed");
        return;
    }

    if (priv.big_image_info.dir_name) {
        free(priv.big_image_info.dir_name);
    }
    priv.big_image_info.dir_name = (char *)malloc(strlen(info->dir_name) + 1);
    if (!priv.big_image_info.dir_name) {
        perror("upload big photo failed");
        return;
    }

    if (priv.big_image_info.type) {
        free(priv.big_image_info.type);
    }
    priv.big_image_info.type = (char *)malloc(strlen(info->type) + 1);
    if (!priv.big_image_info.type) {
        perror("upload big photo failed");
        return;
    }

    src->header.w = info->dsc.header.w;
    src->header.h = info->dsc.header.h;
    src->header.cf = info->dsc.header.cf;
    src->data_size = info->dsc.data_size;
    memcpy((uint8_t *)src->data, info->dsc.data, info->dsc.data_size);
    strcpy(priv.big_image_info.path, info->path);
    strcpy(priv.big_image_info.dir_name, info->dir_name);
    strcpy(priv.big_image_info.type, info->type);
    priv.big_image_info.size = info->size;
    priv.big_image_info.src_width = info->src_width;
    priv.big_image_info.src_height = info->src_height;

    _ui_big_photo_screen_deinit();
    _ui_big_photo_screen_init();
}

static void _ui_small_photo_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(g_base_screen);
    lv_obj_set_pos(obj, 0, lv_pct(12));
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(90) + 1);
    lv_obj_set_style_radius(obj, 0, 0);
    // lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    obj->spec_attr->scroll.y = -priv.photo_list_screen_scroll_y;

    g_small_photo_screen = obj;
}

static void _ui_video_view_screen_deinit(void)
{
    if (g_video_screen) {
        lv_obj_delete(g_video_screen);
        g_video_screen = NULL;
        g_video_bar = NULL;
        g_video_bar_label = NULL;
        g_video_pause_img = NULL;
    }
}
static void _ui_video_view_screen_init(void)
{
    if (g_video_screen) {
        lv_obj_delete(g_video_screen);
        g_video_screen = NULL;
    }

    lv_obj_t *obj = lv_obj_create(lv_layer_sys());
    lv_obj_set_pos(obj, 0, lv_pct(12));
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(88) + 1);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0000), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_0, 0);
    lv_obj_add_event_cb(obj, event_video_view_event_cb, LV_EVENT_PRESSED, NULL);
    g_video_screen = obj;
    lv_obj_add_flag(g_video_screen, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * touch_area = lv_obj_create(obj);
    lv_obj_set_pos(touch_area, lv_pct(0), lv_pct(0));
    lv_obj_set_size(touch_area, lv_pct(90), lv_pct(25));
    lv_obj_set_align(touch_area, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(touch_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_side(touch_area, 0, 0);

    lv_obj_t * bar = lv_bar_create(touch_area);
    lv_bar_set_range(bar, 0, 10000);
    lv_obj_set_size(bar, lv_pct(90), lv_pct(35));
    lv_obj_center(bar);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xf308d6), LV_PART_INDICATOR);
    lv_obj_add_event_cb(bar, event_video_bar_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(bar, event_video_bar_event_cb, LV_EVENT_RELEASED, NULL);
    g_video_bar = bar;

    lv_obj_t * text = lv_label_create(obj);
    lv_obj_set_pos(text, lv_pct(0), lv_pct(-5));
    lv_obj_set_align(text, LV_ALIGN_BOTTOM_MID);
    lv_label_set_text(text, "00:00:23");
    g_video_bar_label = text;

    lv_obj_t * img = lv_image_create(obj);
    lv_image_set_src(img, &img_video_play);
    lv_obj_center(img);
    lv_obj_set_pos(img, 0, lv_pct(-5));
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
    g_video_pause_img = img;
}

void ui_video_view_screen_update(uint32_t total_ms)
{
    _ui_video_view_screen_deinit();
    _ui_video_view_screen_init();
}

static void _ui_small_photo_screen_deinit(void)
{
    if (g_small_photo_screen) {
        priv.photo_list_screen_scroll_y = lv_obj_get_scroll_y(g_small_photo_screen);
        lv_obj_delete(g_small_photo_screen);
        g_small_photo_screen = NULL;
    }
}



int ui_get_need_bulk_delete(void)
{
    return priv.need_bulk_delete;
}

int ui_set_need_bulk_delete(int value)
{
    priv.need_bulk_delete = value;
    return priv.need_bulk_delete;
}

void ui_photo_list_screen_update(void)
{
    _ui_small_photo_screen_deinit();
    _ui_small_photo_screen_init();

    lv_obj_t *scr = g_small_photo_screen;

    Node *next = priv.dirs;
    while (next != NULL) {
        ui_directory_t *dir = &next->data->dir;
        Node *next_photo = dir->list;

        if (next_photo) {
            lv_obj_t *obj = lv_obj_create(scr);
            lv_obj_set_style_radius(obj, 0, 0);
            lv_obj_set_height(obj, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(obj, 0, 0);
            lv_obj_set_style_outline_width(obj, 0, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), 0);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_size(obj, lv_pct(100), lv_pct(12));

            lv_obj_t *label = lv_label_create(obj);
            lv_label_set_text(label, dir->dir_name);
            lv_obj_center(label);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0x0), 0);

            lv_obj_t *obj2 = lv_obj_create(scr);
            lv_obj_set_width(obj2, lv_pct(100));
            lv_obj_set_height(obj2, LV_SIZE_CONTENT);
            lv_obj_set_style_radius(obj2, 0, 0);
            lv_obj_set_style_bg_color(obj2, lv_color_hex(0xffffff), 0);
            lv_obj_set_style_border_side(obj2, LV_BORDER_SIDE_NONE, 0);
            lv_obj_remove_flag(obj2, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(obj2, 0, 0);

            lv_obj_t *sub_obj = lv_obj_create(obj2);
            lv_obj_set_width(sub_obj, lv_pct(84));
            lv_obj_set_height(sub_obj, LV_SIZE_CONTENT);
            lv_obj_center(sub_obj);
            lv_obj_set_style_radius(sub_obj, 0, 0);
            lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(sub_obj, 15, 0);
            lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0xffffff), 0);
            lv_obj_set_style_border_color(sub_obj, lv_color_hex(0x7c7c7c), 0);
            lv_obj_set_style_radius(sub_obj, 10, 0);
            lv_obj_set_flex_flow(sub_obj, LV_FLEX_FLOW_ROW_WRAP);
            lv_obj_set_flex_align(sub_obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);


            while (next_photo != NULL) {
                ui_photo_t *photo = &next_photo->data->photo;
                {
                    lv_obj_t *sub_obj2 = lv_obj_create(sub_obj);
                    lv_obj_set_size(sub_obj2, 128, 143);
                    lv_obj_set_style_radius(sub_obj2, 0, 0);
                    lv_obj_remove_flag(sub_obj2, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_set_style_pad_all(sub_obj2, 0, 0);
                    lv_obj_set_style_bg_color(sub_obj2, lv_color_hex(0xffffff), 0);
                    lv_obj_set_style_border_side(sub_obj2, LV_BORDER_SIDE_NONE, 0);
                    lv_obj_remove_flag(sub_obj2, LV_OBJ_FLAG_SCROLLABLE);

                    lv_obj_t *img = lv_image_create(sub_obj2);
                    lv_image_set_src(img, photo->img_dsc);
                    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
                    lv_obj_add_event_cb(img, event_touch_small_image_cb, LV_EVENT_SHORT_CLICKED, photo);
                    lv_obj_add_event_cb(img, event_touch_small_image_cb, LV_EVENT_LONG_PRESSED, photo);
                    lv_obj_set_user_data(img, dir->dir_name);

                    lv_obj_t *sub_obj3 = lv_obj_create(img);
                    lv_obj_align(sub_obj3, LV_ALIGN_DEFAULT, 5, 5);
                    lv_obj_set_size(sub_obj3, 5, 5);
                    lv_obj_set_style_radius(sub_obj3, 0, 0);
                    lv_obj_remove_flag(sub_obj3, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_set_style_radius(sub_obj3, 90, 0);
                    lv_obj_set_style_pad_all(sub_obj3, 0, 0);
                    lv_obj_set_style_bg_color(sub_obj3, lv_color_hex(0xff0000), 0);
                    lv_obj_set_style_border_side(sub_obj3, LV_BORDER_SIDE_NONE, 0);
                    lv_obj_remove_flag(sub_obj3, LV_OBJ_FLAG_SCROLLABLE);

                    if (ui_get_need_bulk_delete()) {
                        if (photo->is_touch) {
                            lv_obj_set_style_bg_color(sub_obj3, lv_color_hex(0xff0000), 0);
                            lv_obj_set_style_bg_opa(sub_obj3, LV_OPA_100, 0);
                        } else {
                            lv_obj_set_style_bg_color(sub_obj3, lv_color_hex(0xffffff), 0);
                            lv_obj_set_style_bg_opa(sub_obj3, LV_OPA_100, 0);
                        }
                    } else {
                        lv_obj_set_style_bg_color(sub_obj3, lv_color_hex(0xffffff), 0);
                        lv_obj_set_style_bg_opa(sub_obj3, LV_OPA_0, 0);
                    }

                    if (photo->is_video) {
                        lv_obj_t *pause_img = lv_image_create(img);
                        lv_obj_center(pause_img);
                        lv_image_set_src(pause_img, &img_video_play);
                    }
                }
                next_photo = next_photo->next;
            }
        }
        next = next->next;
    }
}

void ui_photo_add_dir(char *dir_name)
{
    NodeItem data = {0};
    strcpy(data.dir.dir_name, dir_name);

    if (!priv.dirs) {
        priv.dirs = createNode(&data);
    } else {
        appendNode(&priv.dirs, &data);
    }
}

void ui_photo_del_dir(char *dir_name)
{
    deleteDirNode(&priv.dirs, dir_name);
}

void ui_photo_add_photo(char *dir_name, char *path, lv_image_dsc_t *dsc, bool is_video)
{
    Node *head = priv.dirs;
    if (!head) {
        printf("dir head is not create!\r\n");
        return;
    }

    Node *dir_node = selectDirNode(head, dir_name);
    if (dir_node == NULL) {
        printf("Not found dir:%s\r\n", dir_name);
        return;
    }

    NodeItem data = {0};

    data.photo.path = (char *)malloc(strlen(path) + 1);
    if (!data.photo.path) {
        perror("add photo failed!\r\n");
        return;
    }

    data.photo.img_dsc = (lv_image_dsc_t *)malloc(sizeof(lv_image_dsc_t));
    if (!data.photo.img_dsc) {
        perror("add photo failed!\r\n");
        free(data.photo.path);
        return;
    }

    strcpy(data.photo.path, path);
    memcpy(data.photo.img_dsc, dsc, sizeof(lv_image_dsc_t));
    data.photo.img_dsc->data = (uint8_t *)malloc(dsc->data_size);
    if (!data.photo.img_dsc->data) {
        perror("add photo failed!\r\n");
        free(data.photo.img_dsc);
        free(data.photo.path);
        return;
    }
    data.photo.is_video = is_video;
    memcpy((uint8_t *)data.photo.img_dsc->data, dsc->data, dsc->data_size);

    appendNode(&dir_node->data->dir.list, &data);
}

void ui_photo_del_photo(char *dir_name, char *path)
{
    Node *head = priv.dirs;

    Node *dir_node = selectDirNode(head, dir_name);
    if (dir_node == NULL) {
        printf("Not found dir:%s\r\n", dir_name);
        return;
    }

    deletePicNode(&dir_node->data->dir.list, path);
}

void ui_photo_clear_all_photo_flag(void)
{
    Node* temp = priv.dirs;

    while (temp != NULL) {
        if (temp->data) {
            Node *photo_node = temp->data->dir.list;
            while (photo_node != NULL) {
                photo_node->data->photo.is_touch = 0;
                photo_node = photo_node->next;
            }
        }

        temp = temp->next;
    }
}

void ui_bulk_delete_path_init(void)
{
    priv.bulk_delete_iter_dir = priv.dirs;
    priv.bulk_delete_iter_photo = NULL;
}

int ui_bulk_delete_path_iter(char **dirname, char **img_path)
{

    Node* temp = priv.dirs;
    while (temp != NULL) {
        if (temp->data) {
            Node *photo_node = temp->data->dir.list;
            while (photo_node != NULL) {
                if (photo_node->data->photo.is_touch)
                {
                    if (dirname) {
                        *dirname = temp->data->dir.dir_name;
                    }

                    if (img_path) {
                        *img_path = photo_node->data->photo.path;
                    }
                    photo_node->data->photo.is_touch = 0;
                    return 0;
                }
                photo_node = photo_node->next;
            }
        }

        temp = temp->next;
    }

    return -1;
}

void ui_photo_print(void)
{
    Node* temp = priv.dirs;

    while (temp != NULL) {
        if (temp->data) {
            Node *photo_node = temp->data->dir.list;
            while (photo_node != NULL) {
                printf("photo path:%s\r\n", photo_node->data->photo.path);
                photo_node = photo_node->next;
            }
        }

        temp = temp->next;
    }
}

void ui_all_screen_init(void)
{
    priv_init();

#if 1
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(NULL, LV_COLOR_FORMAT_ARGB8888);
    lv_screen_load(lv_layer_top());
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(lv_layer_sys(), LV_OBJ_FLAG_SCROLLABLE);
#else
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
#endif

    _ui_base_screen_init();
    _ui_upper_screen_init();
    _ui_lower_screen_init();
    _ui_right_screen_init();
    _ui_big_photo_screen_init();
    _ui_small_photo_screen_init();
    _ui_video_view_screen_init();
    _ui_left_right_init();
#if 0
    lv_image_dsc_t *dsc = &test_img_128x128;
    char *dir1 = "2003-03-05";
    ui_photo_add_dir(dir1);
    ui_photo_add_photo(dir1, "pic1", dsc);
    ui_photo_add_photo(dir1, "pic2", dsc);
    ui_photo_add_photo(dir1, "pic3", dsc);
    char *dir2 = "2013-03-15";
    ui_photo_add_dir(dir2);

    ui_photo_add_photo(dir2, "pic4", dsc);
    ui_photo_add_photo(dir2, "pic5", dsc);
    ui_photo_add_photo(dir2, "pic6", dsc);
    ui_photo_add_photo(dir2, "pic9", dsc);
    ui_photo_add_photo(dir2, "pic7", dsc);
    ui_photo_add_photo(dir2, "pic65", dsc);
    ui_photo_add_photo(dir2, "pic546", dsc);
    ui_photo_add_photo(dir2, "pic4564", dsc);
    ui_photo_add_photo(dir2, "pic354345", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    char *dir3 = "2026-04-25";
    ui_photo_add_dir(dir3);
    ui_photo_add_photo(dir3, "pic7", dsc);
    ui_photo_add_photo(dir3, "pic8", dsc);
    ui_photo_add_photo(dir3, "pic9", dsc);

    ui_photo_print();

    // ui_photo_del_dir(dir2);
    // ui_photo_print();

    // ui_photo_del_photo(dir1, "pic1");
    // ui_photo_del_photo(dir3, "pic8");
    ui_photo_print();

    // ui_photo_del_dir(dir1);
    ui_photo_list_screen_update();
#endif
}

void ui_all_screen_deinit(void)
{
    lv_image_dsc_t *src = (lv_image_dsc_t *)&priv.big_image_info.dsc;
    if (src->data) {
        free((uint8_t *)src->data);
        src->data = NULL;
    }
}

void ui_set_video_bar_s(double curr_s, double total_s)
{
    if (g_video_bar_label) {
        lv_label_set_text_fmt(g_video_bar_label, "%.2d:%.2d:%.2d/%.2d:%.2d:%.2d",
            (uint32_t)curr_s / 3600, ((uint32_t)curr_s % 3600) / 60, (uint32_t)curr_s % 60, (uint32_t)total_s / 3600, ((uint32_t)total_s % 3600) / 60, (uint32_t)total_s % 60);
    }

    if (g_video_bar) {
        int max_value = lv_bar_get_max_value(g_video_bar);
        int min_value = lv_bar_get_min_value(g_video_bar);
        int new_value = ((double)curr_s / total_s) * (max_value - min_value) + min_value;
        lv_bar_set_value(g_video_bar, new_value, LV_ANIM_OFF);
    }
}

void ui_clear_video_bar()
{
    if (g_video_bar) {
        lv_bar_set_value(g_video_bar, 0, LV_ANIM_OFF);
    }
}