#ifndef RECEIVER_H_
#define RECEIVER_H_

#define MY_ADDRESS 0x20

void InitReceiverTimer(void);

void EnableReceiver(void);

void DisableReceiver(void);

void ProcessReceivedMessage(void);

void ClearReceiverArr(void);

void InitReceiverArr(void);

void ResetTIM4Cnt(void);


#endif /* RECEIVER_H_ */
