#ifndef HC_SR04_DRIVER_H_
#define HC_SR04_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void hc_sr04_driver_init(void);

float hc_sr04_driver_get_dist(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* HC_SR04_DRIVER_H_ */
