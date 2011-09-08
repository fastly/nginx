/* vim:set ft=c ts=4 sw=4 et fdm=marker: */
#include "ngx_http_lua_conf.h"
#include "ngx_http_lua_util.h"


static void ngx_http_lua_cleanup_vm(void *data);
static char * ngx_http_lua_init_vm(ngx_conf_t *cf, ngx_http_lua_main_conf_t *lmcf);


void *
ngx_http_lua_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_lua_main_conf_t *lmcf;

    lmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_lua_main_conf_t));
    if (lmcf == NULL) {
        return NULL;
    }

    /* set by ngx_pcalloc:
     *      lmcf->lua = NULL;
     *      lmcf->lua_path = { 0, NULL };
     *      lmcf->lua_cpath = { 0, NULL };
     */

    dd("NginX Lua module main config structure initialized!");

    return lmcf;
}


char *
ngx_http_lua_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_lua_main_conf_t *lmcf = conf;

    if (lmcf->lua == NULL) {
        if (ngx_http_lua_init_vm(cf, lmcf) != NGX_CONF_OK) {
            ngx_conf_log_error(NGX_ERROR, cf, 0, "Failed to initialize Lua VM!");
            return NGX_CONF_ERROR;
        }

        dd("Lua VM initialized!");
    }

    return NGX_CONF_OK;
}


void*
ngx_http_lua_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_lua_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_lua_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    /* set by ngx_pcalloc:
     *      conf->src = { 0, NULL };
     */

    conf->force_read_body = NGX_CONF_UNSET;

    return conf;
}


char*
ngx_http_lua_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_lua_loc_conf_t *prev = parent;
    ngx_http_lua_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->src, prev->src, "");
    ngx_conf_merge_value(conf->force_read_body, prev->force_read_body, 0);

    return NGX_CONF_OK;
}


static void
ngx_http_lua_cleanup_vm(void *data)
{
    lua_State *lua = data;

    if (lua != NULL) {
        lua_close(lua);

        dd("Lua VM closed!");
    }
}


static char*
ngx_http_lua_init_vm(ngx_conf_t *cf, ngx_http_lua_main_conf_t *lmcf)
{
    ngx_pool_cleanup_t *cln;

    /* add new cleanup handler to config mem pool */
    cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        return NGX_CONF_ERROR;
    }

    /* create new Lua VM instance */
    lmcf->lua = ngx_http_lua_new_state(cf, lmcf);
    if (lmcf->lua == NULL) {
        return NGX_CONF_ERROR;
    }

    /* register cleanup handler for Lua VM */
    cln->handler = ngx_http_lua_cleanup_vm;
    cln->data = lmcf->lua;

    return NGX_CONF_OK;
}

