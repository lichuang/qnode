/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QCORE_H__
#define __QCORE_H__

typedef struct qactor_t       qactor_t; 
typedef struct qconfig_t      qconfig_t; 
typedef struct qengine_t      qengine_t; 
typedef struct qlog_t         qlog_t;
typedef struct qmailbox_t     qmailbox_t;
typedef struct qmsg_t         qmsg_t;
typedef struct qsignal_t      qsignal_t;
typedef struct qsocket_t      qsocket_t;
typedef struct qthread_log_t  qthread_log_t;
typedef struct qvalue_t       qvalue_t;
typedef struct qworker_t      qworker_t;

#define  QOK          0
#define  QERROR      -1

#define  QINVALID_ID  0
#endif  /* __QCORE_H__ */
