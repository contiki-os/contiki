diff --git a/cpu/avr/radio/rf230bb/rf230bb.c b/cpu/avr/radio/rf230bb/rf230bb.c
index 53959d1..b9f2262 100644
--- a/cpu/avr/radio/rf230bb/rf230bb.c
+++ b/cpu/avr/radio/rf230bb/rf230bb.c
@@ -544,7 +544,7 @@ rf230_set_promiscuous_mode(bool isPromiscuous) {
 #if RF230_CONF_AUTOACK
     is_promiscuous = isPromiscuous;
 /* TODO: Figure out when to pass promisc state to 802.15.4 */
-//    radio_set_trx_state(is_promiscuous?RX_ON:RX_AACK_ON);
+    radio_set_trx_state(is_promiscuous?RX_ON:RX_AACK_ON);
 #endif
 }
 
@@ -1392,6 +1392,25 @@ PROCESS_THREAD(rf230_process, ev, data)
     /* Restore interrupts. */
     HAL_LEAVE_CRITICAL_REGION();
     PRINTF("rf230_read: %u bytes lqi %u\n",len,rf230_last_correlation);
+
+    if(is_promiscuous) {
+      uint8_t i;
+      unsigned const char * rxdata = packetbuf_dataptr();
+      /* Print magic */
+      putchar(0xC1);
+      putchar(0x1F);
+      putchar(0xFE);
+      putchar(0x72);
+      /* Print version */
+      putchar(0x01);
+      /* Print CMD == frame */
+      putchar(0x00);
+      putchar(len+3);
+
+      for (i=0;i<len;i++)  putchar(rxdata[i]);
+      printf("\n");
+    }
+
 #if DEBUG>1
      {
         uint8_t i;
