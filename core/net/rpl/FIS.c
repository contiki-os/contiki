#include <stdint.h>
#include "FIS.h"

uint16_t min(uint16_t a, uint16_t b){
    if(a>b) return b;
    return a;
}
uint16_t max(uint16_t a, uint16_t b){
    if(a>b) return a;
    return b;
}

uint16_t up(uint16_t x1, uint16_t x2, uint16_t x){
    return 100 * (x - x1)/(x2 - x1);
}
uint16_t down(uint16_t x1, uint16_t x2, uint16_t x){
    return 100 * (x - x2)/(x1 - x2);
}

uint16_t first_t_norm(uint16_t a, uint16_t b, uint16_t x){
    if (x < a) return 100;
    if (x >= a && x <= b) return down(a, b, x);
    return 0;
}

uint16_t t_norm(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t x){
    if (x >= a && x < b) return up(a, b, x);
    if (x >= b && x <= c) return 100;
    if (x > c && x <= d) return down(c, d, x);
    return 0;
}

uint16_t last_t_norm(uint16_t c, uint16_t d, uint16_t x){
    if (x > c && x < d) return up(c, d, x);
    if (x >= d) return 100;
    return 0;
}


//QoS = FIS(ETX, Delay, HopCount)
uint8_t qos(uint16_t etx, uint16_t delay, uint16_t hc){
    uint16_t setx = etx_short(etx);
    uint16_t aetx = etx_avg(etx);
    uint16_t letx = etx_long(etx);

    uint16_t sdly = dly_small(delay);
    uint16_t adly = dly_avg(delay);
    uint16_t ldly = dly_high(delay);

    uint16_t nhc = hc_near(hc);
    uint16_t ahc = hc_avg(hc);
    uint16_t fhc = hc_far(hc);
    
    // if etx is short and delay is small and hop count is near then QoS is very fast
    uint16_t vfqos = min(setx, min(sdly, nhc));
    // if (etx is short and delay is average and hc is near) or ...
    uint16_t fqos = max(
        max(
            max(min(setx, min(adly, nhc)),min(setx, min(sdly, ahc))),
            max(min(setx, min(adly, ahc)),min(setx, min(sdly, fhc)))
        ),
        max(
            max(min(setx, min(adly, fhc)),min(aetx, min(sdly, nhc))),
            max(min(aetx, min(sdly, ahc)),min(aetx, min(sdly, fhc)))
        )
    );
    uint16_t aqos = max(
        max(
            max(
                max(min(setx, min(ldly, nhc)),min(setx, min(ldly, ahc))),
                max(min(setx, min(ldly, fhc)),min(aetx, min(adly, nhc)))
            ),
            max(
                max(min(aetx, min(adly, ahc)),min(aetx, min(adly, fhc))),
                max(min(letx, min(sdly, nhc)),min(letx, min(sdly, ahc)))
            )
        ),
        min(letx, min(sdly, fhc))
    );
    uint16_t sqos = max(
        max(
            max(min(aetx, min(ldly, nhc)),min(aetx, min(ldly, ahc))),
            max(min(aetx, min(ldly, fhc)),min(letx, min(adly, nhc)))
        ),
        max(
            max(min(letx, min(ldly, nhc)),min(letx, min(adly, ahc))),
            max(min(letx, min(ldly, ahc)),min(letx, min(adly, fhc)))
        )
    );
    uint16_t vsqos = min(letx, min(ldly, fhc));
    return (vfqos*90 + fqos*70 + aqos*50 + sqos*30 + vsqos*10)/(vfqos + fqos + aqos + sqos + vsqos);
}

//Quality = FIS(QoS, Energy)
uint8_t quality(uint16_t q, uint16_t e){
    uint16_t vsqos = qos_very_slow(q);
    uint16_t sqos = qos_slow(q);
    uint16_t aqos = qos_avg(q);
    uint16_t fqos = qos_fast(q);
    uint16_t vfqos = qos_very_fast(q);

    uint16_t leng = energy_low(e);
    uint16_t meng = energy_medium(e);
    uint16_t feng = energy_full(e);

    uint16_t awf = min(vsqos, leng);
    uint16_t bad = max(min(vsqos, meng), min(sqos, leng));
    uint16_t deg = max(min(sqos, meng), min(aqos, leng));
    uint16_t avg = max(
        max(
            max(min(vsqos, feng), min(sqos, feng)),
            max(min(aqos, meng), min(fqos, leng))
            ),
        min(vfqos, leng)
    );
    uint16_t accep = max(min(aqos, feng), min(fqos, meng));
    uint16_t good = max(min(fqos, feng), min(vfqos, meng));
    uint16_t exc = min(vfqos, feng);

    return (awf*9 + bad*21 + deg*35 + avg*49 + accep*63 + good*77 + exc*91)/(awf + bad + deg + avg + accep + good + exc);
}
// code to test the FIS
//int main(){
//    uint16_t q;
//    uint16_t qual;
//    unsigned short etx[] = {5, 2, 10, 15, 3, 15, 4};
//    unsigned short delay[] = {1500, 1000, 700, 500, 700, 2500, 122};
//    unsigned short hc[] = {3, 2, 1, 2, 1, 3, 75};
//    unsigned short energy[] = {200, 200, 225, 200, 250, 75, 254};
//    for(int i = 0; i<7; i++){
//        q = qos(etx[i], delay[i], hc[i]);
//        qual = quality(q, energy[i]);
//        printf("etx\tdelay\thc\tenergy\tQoS\tQuality\n");
//        printf("%i\t%i\t%i\t%i\t%i\t%i\n\n", etx[i], delay[i], hc[i], energy[i], q, qual);
//    }
//}
