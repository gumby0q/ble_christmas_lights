
#ifndef BLE_PWM_H__
#define BLE_PWM_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_PWM_BLE_OBSERVER_PRIO 2
#define BLE_PWM_UUID_BASE                        \
  { 0xa8, 0xb4, 0xc9, 0x4a, 0xad, 0x6c, 0x11, 0xe6, \
    0x9a, 0xd8, 0x5d, 0x07, 0xa0, 0x9d, 0x94, 0x6b }

#define BLE_PWM_UUID_SERVICE 0x3144
#define BLE_PWM_UUID_COM_CHAR 0x3145
#define BLE_PWM_UUID_VALUE_CHAR 0x3146

//#define BLE_PWM_DEF(_name)       \
//  static ble_bmi160_t _name;        \
//  NRF_SDH_BLE_OBSERVER(_name##_obs, \
//      BLE_PWM_BLE_OBSERVER_PRIO, \
//      ble_bmi160_on_ble_evt, &_name)



/**@brief   Macro for defining a ble_pwm instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_PWM_DEF(_name)                                                                          \
static ble_pwm_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_PWM_BLE_OBSERVER_PRIO,                                                     \
                     ble_pwm_on_ble_evt, &_name)


/**@brief PWM Service event type. */
typedef enum
{
    BLE_PWM_EVT_NOTIFICATION_ENABLED,   /**< PWM value notification enabled event. */
    BLE_PWM_EVT_NOTIFICATION_DISABLED,   /**< PWM value notification disabled event. */
    BLE_PWM_EVT_TX_COMPLETE
} ble_pwm_evt_type_t;

/**@brief PWM Service event. */
typedef struct
{
    ble_pwm_evt_type_t evt_type;        /**< Type of event. */
} ble_pwm_evt_t;

// Forward declaration of the ble_pwm_t type.
typedef struct ble_pwm_s ble_pwm_t;

/**@brief PWM Service event handler type. */
typedef void (*ble_pwm_evt_handler_t) (ble_pwm_t * p_pwm, ble_pwm_evt_t * p_evt);

/**@brief PWM Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_pwm_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the PWM Service. */
    bool                          support_notification;           /**< TRUE if notification of PWM Level measurement is supported. */
    ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the PWM Level characteristic */
    uint8_t                       initial_pwm_level;             /**< Initial pwm level */
    ble_srv_cccd_security_mode_t  pwm_level_char_attr_md;     /**< Initial security level for pwm characteristics attribute */
    ble_gap_conn_sec_mode_t       pwm_level_report_read_perm; /**< Initial security level for pwm report read attribute */
} ble_pwm_init_t;

/**@brief PWM Service structure. This contains various status information for the service. */
struct ble_pwm_s
{
    ble_pwm_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the PWM Service. */
    uint16_t                      service_handle;                 /**< Handle of PWM Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      pwm_level_handles;          /**< Handles related to the PWM Level characteristic. */
    uint16_t                      report_ref_handle;              /**< Handle of the Report Reference descriptor. */
    uint8_t                       pwm_level_last;             /**< Last PWM Level measurement passed to the PWM Service. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                          is_notification_supported;      /**< TRUE if notification of PWM Level is supported. */
    uint8_t                       uuid_type;
};

//struct ble_pwm_s {
//  ble_pwm_evt_handler_t     evt_handler;
//  uint16_t                  service_handle;
//  ble_gatts_char_handles_t  bmi_handles;
//  uint8_t                   uuid_type;
//  uint16_t                  conn_handle;
//  uint8_t                   max_pwm_len;
//};


/**@brief Function for initializing the PWM Service.
 *
 * @param[out]  p_pwm       PWM Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_pwm_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_pwm_init(ble_pwm_t * p_pwm, const ble_pwm_init_t * p_pwm_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the PWM Service.
 *
 * @note For the requirements in the PWM specification to be fulfilled,
 *       ble_pwm_pwm_level_update() must be called upon reconnection if the
 *       pwm level has changed while the service has been disconnected from a bonded
 *       client.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   PWM Service structure.
 */
void ble_pwm_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief Function for updating the pwm level.
 *
 * @details The application calls this function after having performed a pwm measurement. If
 *          notification has been enabled, the pwm level characteristic is sent to the client.
 *
 * @note For the requirements in the PWM specification to be fulfilled,
 *       this function must be called upon reconnection if the pwm level has changed
 *       while the service has been disconnected from a bonded client.
 *
 * @param[in]   p_pwm          PWM Service structure.
 * @param[in]   pwm_level  New pwm measurement value (in percent of full capacity).
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_pwm_level_update(ble_pwm_t * p_pwm, uint8_t pwm_level);


#ifdef __cplusplus
}
#endif

#endif // BLE_PWM_H__

/** @} */
