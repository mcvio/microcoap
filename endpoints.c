#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "coap.h"

static char light = '0';
static int voltage = 0;
static int current = 0;
static char buffer[10] = "";

const uint16_t rsplen = 1500;
static char rsp[1500] = "";
void build_rsp(void);

char *itoa(int num, char *str)
{
    if (str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

#ifdef ARDUINO
#include "Arduino.h"
static int led = 6;
void endpoint_setup(void)
{
    pinMode(led, OUTPUT);
    build_rsp();
}
#else
#include <stdio.h>
void endpoint_setup(void)
{
    build_rsp();
}
#endif

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static const coap_endpoint_path_t path_light = {1, {"light"}};
static const coap_endpoint_path_t path_voltage = {2, {"353161072767299" , "voltage"}};
static const coap_endpoint_path_t path_current = {2, {"353161072767299" , "current"}};
static const coap_endpoint_path_t path_data = {2, {"353161072767299" , "data"}};

static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    printf("Send light status : %c\r\n", light);
    return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
        return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
    if (inpkt->payload.p[0] == '1')
    {
        light = '1';
        printf("LIGHT is now set : ON\n");
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    else
    {
        light = '0';
        printf("LIGHT is now set : OFF\n");
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}
static int handle_get_voltage(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (itoa(voltage, buffer)) {
        return coap_make_response(scratch, outpkt, buffer, strlen(buffer), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    else
    {
        buffer[0] = 'E';
        buffer[1] = 0x00;
        return coap_make_response(scratch, outpkt, (const uint8_t *)&buffer, strlen(buffer), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}

static int handle_get_current(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (itoa(current, buffer)) {
        return coap_make_response(scratch, outpkt, buffer, strlen(buffer), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    else
    {
        buffer[0] = 'E';
        buffer[1] = 0x00;
        return coap_make_response(scratch, outpkt, (const uint8_t *)&buffer, strlen(buffer), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}


// static int handle_put_voltage(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
// {
//     if (inpkt->payload.len == 0){
//         return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
//     }else
//     {
//         printf("You send %s voltage\r\n",inpkt->payload.p);
//         return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
//     }
// }


static int handle_put_data(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
     if (inpkt->payload.len == 0) {
        voltage = 0;
        current = 0;
        return coap_make_response(scratch, outpkt, "NotOK", 5, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    } else if (inpkt->payload.len != 6) {
        voltage = 0;
        current = 0;
        return coap_make_response(scratch, outpkt, "NotLength", 5, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    } else {
        printf("You send %s data\r\n", inpkt->payload.p);
        voltage = (inpkt->payload.p[0]-0x30)*100+(inpkt->payload.p[1]-0x30)*10+(inpkt->payload.p[2]-0x30);
        current = (inpkt->payload.p[3]-0x30)*100+(inpkt->payload.p[4]-0x30)*10+(inpkt->payload.p[5]-0x30);;

        printf("\twhere voltage : %d\r\n", voltage);
        printf("\twhere current : %d\r\n", current);
        
        printf("You send %s voltage\r\n", inpkt->payload.p);
        return coap_make_response(scratch, outpkt, "OK", 2, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}


const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},
    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},
    {COAP_METHOD_GET, handle_get_voltage, &path_voltage, "ct=0"},
    //{COAP_METHOD_PUT, handle_put_voltage, &path_voltage, "ct=0"},
    {COAP_METHOD_GET, handle_get_current, &path_current, "ct=0"},
    //{COAP_METHOD_PUT, handle_put_voltage, &path_current, "ct=0"},
    //{COAP_METHOD_GET, handle_get_data, &path_data, "ct=0"},
    {COAP_METHOD_PUT, handle_put_data, &path_data, "ct=0"},
    {(coap_method_t)0, NULL, NULL, NULL}
};

void build_rsp(void)
{
    uint16_t len = rsplen;
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while(NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}

