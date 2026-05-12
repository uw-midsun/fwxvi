
#include "generic_wrapper.h"

widget_t* create_widget(widget_config_t* config){
    if (!config) return NULL;

    widget_t *widget=malloc(sizeof(widget_t));
    if (!widget) return NULL;
    
    widget->config = config;
    widget->obj = lv_image_create(config->parent);
    if (!widget->obj) {
            free(widget);
            return NULL;
    }

    lv_obj_set_pos(widget->obj, config->x, config->y);
    lv_obj_set_size(widget->obj, config->width, config->height);

    if (config->img_src) {
        lv_image_set_src(widget->obj, config->img_src);
    }

    lv_obj_set_style_bg_color(widget->obj, config->color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_COVER, LV_PART_MAIN);

    return widget;

}

void set_image(const lv_image_dsc_t* img, widget_t *widget){
    if (widget->obj && widget){
        lv_image_set_src(widget->obj, img);
    }
}

void set_color(const lv_color_t color, widget_t *widget){
    if (widget && widget->obj){
        lv_obj_set_style_bg_color(widget->obj, color, LV_PART_MAIN);
    }
}

