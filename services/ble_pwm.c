
#include "ble_pwm.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_log.h"

#define MAX_PACKET_LEN (int8_t)12
#define INVALID_BATTERY_LEVEL 255


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_pwm       PWM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_pwm_t * p_pwm, ble_evt_t const * p_ble_evt)
{
    p_pwm->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_pwm       PWM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_pwm_t * p_pwm, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_pwm->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the Write event.
 *
 * @param[in]   p_pwm       PWM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_pwm_t * p_pwm, ble_evt_t const * p_ble_evt)
{
    if (!p_pwm->is_notification_supported)
    {
        return;
    }

    NRF_LOG_INFO("on_write %d", p_pwm->is_notification_supported);
    
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    NRF_LOG_INFO("on_write01 %d", p_evt_write->len);
    NRF_LOG_INFO("on_write02 %d %d", p_evt_write->handle, p_pwm->pwm_level_handles.cccd_handle);
    NRF_LOG_INFO("on_write03 %d %d", p_evt_write->handle, p_pwm->pwm_level_handles.value_handle);

    if (   (p_evt_write->handle == p_pwm->pwm_level_handles.value_handle)
        && (p_evt_write->len == 3)
        && (p_pwm->pwm_write_handler != NULL))
    {

        p_pwm->pwm_write_handler(p_ble_evt->evt.gap_evt.conn_handle, p_pwm, p_evt_write->data);
    }

    if (    (p_evt_write->handle == p_pwm->pwm_level_handles.cccd_handle)
        &&  (p_evt_write->len == 2))
    {
        if (p_pwm->evt_handler == NULL)
        {
            return;
             NRF_LOG_INFO("on_write1 %d", p_pwm->is_notification_supported);
        }

        ble_pwm_evt_t evt;

         NRF_LOG_INFO("on_write2 %d", p_evt_write->data[0]);

        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            evt.evt_type = BLE_PWM_EVT_NOTIFICATION_ENABLED;
        }
        else
        {
            evt.evt_type = BLE_PWM_EVT_NOTIFICATION_DISABLED;
        }

        // CCCD written, call application event handler.
        p_pwm->evt_handler(p_pwm, &evt);
    }
}


void ble_pwm_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_pwm_t * p_pwm = (ble_pwm_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_pwm, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_pwm, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_pwm, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding the PWM Level characteristic.
 *
 * @param[in]   p_pwm        PWM Service structure.
 * @param[in]   p_pwm_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pwm_level_char_add(ble_pwm_t * p_pwm, const ble_pwm_init_t * p_pwm_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             initial_pwm_level[MAX_PACKET_LEN];
    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
    uint8_t             init_len;

    // Add PWM Level characteristic
//    if (p_pwm->is_notification_supported)
//    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // According to PWM_SPEC_V10, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_pwm_init->pwm_level_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
//    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
//    char_md.char_props.notify = (p_pwm->is_notification_supported) ? 1 : 0;
    char_md.char_props.notify = 1;
    char_md.char_props.write = 1; // custom
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    //char_md.p_cccd_md         = (p_pwm->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;


    ble_uuid.type = p_pwm->uuid_type;
    ble_uuid.uuid = BLE_PWM_UUID_VALUE_CHAR;
//    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_LEVEL_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_pwm_init->pwm_level_char_attr_md.read_perm;
    attr_md.write_perm = p_pwm_init->pwm_level_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = MAX_PACKET_LEN;

    initial_pwm_level[0] = p_pwm_init->initial_pwm_level;
//    NRF_LOG_INFO("teststsststst %d", initial_pwm_level);

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = MAX_PACKET_LEN;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_PACKET_LEN;
    attr_char_value.p_value   = initial_pwm_level;

    err_code = sd_ble_gatts_characteristic_add(p_pwm->service_handle, 
                                               &char_md,
                                               &attr_char_value,
                                               &p_pwm->pwm_level_handles);
        
    NRF_LOG_INFO("teststsststst err_code %d", err_code);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


uint32_t ble_pwm_init(ble_pwm_t * p_pwm, const ble_pwm_init_t * p_pwm_init)
{
    if (p_pwm == NULL || p_pwm_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_pwm->pwm_write_handler               = p_pwm_init->pwm_write_handler;

    p_pwm->evt_handler               = p_pwm_init->evt_handler;
    p_pwm->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_pwm->is_notification_supported = p_pwm_init->support_notification;
    p_pwm->pwm_level_last            = INVALID_BATTERY_LEVEL;

    ble_uuid128_t base_uuid = {BLE_PWM_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_pwm->uuid_type);
    if (err_code != NRF_SUCCESS) {
      return err_code;
    }

    // Add service
    //  BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_SERVICE);
    ble_uuid.type = p_pwm->uuid_type;
    ble_uuid.uuid = BLE_PWM_UUID_SERVICE;


    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_pwm->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add pwm level characteristic
    return pwm_level_char_add(p_pwm, p_pwm_init);
}


uint32_t ble_pwm_level_update(ble_pwm_t * p_pwm, uint8_t pwm_level)
{
    if (p_pwm == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

//    if (pwm_level != p_pwm->pwm_level_last)
//    {
        // Initialize value struct.
        memset(&gatts_value, 0, sizeof(gatts_value));

        gatts_value.len     = sizeof(uint8_t);
        gatts_value.offset  = 0;
        gatts_value.p_value = &pwm_level;

        // Update datapwme.
        err_code = sd_ble_gatts_value_set(p_pwm->conn_handle,
                                          p_pwm->pwm_level_handles.value_handle,
                                          &gatts_value);
        if (err_code == NRF_SUCCESS)
        {
            // Save new pwm value.
            p_pwm->pwm_level_last = pwm_level;

        }
        else
        {
            return err_code;
        }

        // Send value if connected and notifying.
        if ((p_pwm->conn_handle != BLE_CONN_HANDLE_INVALID) && p_pwm->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));

            hvx_params.handle = p_pwm->pwm_level_handles.value_handle;
            hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = gatts_value.offset;
            hvx_params.p_len  = &gatts_value.len;
            hvx_params.p_data = gatts_value.p_value;

            err_code = sd_ble_gatts_hvx(p_pwm->conn_handle, &hvx_params);
                        NRF_LOG_INFO("ahahah send %d", pwm_level);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
//    }

    return err_code;
}
