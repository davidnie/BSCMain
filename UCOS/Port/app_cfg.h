/*
 * �ļ�·����
 *      UCOSII/Ports/app_cfg.h
 * �޸�ʱ�䣺
 *      2013-6-20
 * �汾��
 *      V 1.0
 * ���ߣ�
 *      Eamonn (c) www.eamonning.com
 * ��ϵ��
 *      i@eamonning.com
 */

#ifndef  _UCOSII_PORTS_UCOSII_CFG_H_
#define  _UCOSII_PORTS_UCOSII_CFG_H_

//�û����õ�������ȼ�
#define		OS_USER_PRIO_LOWEST		(OS_LOWEST_PRIO-4)

//�û����õ�������ȼ�
#define		OS_USER_PRIO_HIGHEST	4

//��ȡ���ȼ�
#define		OS_USER_PRIO_GET(N)		(OS_USER_PRIO_HIGHEST+N)



#endif
