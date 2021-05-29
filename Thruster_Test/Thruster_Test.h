#pragma once

#include <QtWidgets/QMainWindow>
#include <qtcpsocket.h>
#include "ui_Thruster_Test.h"
#include <qcategoryaxis.h>
#include <QtCharts/qchartglobal.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/qvalueaxis.h>
#define IN_FRAME_SIZE 47
#define IN_VIBRO_COUNT 20

using namespace QtCharts;

class Thruster_Test : public QMainWindow
{
	Q_OBJECT
	
	
public:
	Thruster_Test(QWidget *parent = Q_NULLPTR);
	
	
	
private slots:
	void connectClick(void);
	void startClick(void);
	void stopClick(void);
	void tensoClick(void);
	void pwmClick(void);
	void offsetClick(void);
	void pauseClick(void);
	void throtClick(void);
	void autoTestClick(void);
	void frameRead(void);
	void conOpened(void);
	void conClosed(void);

	void refreshChart();
private:

	int pauseState = 0;

	/*Structure of sending data*/
	struct outParameters
	{
		int riseTime = 0;
		int stayTime = 0;
		int fallTime = 0;
		int offsetDuty = 0;
		enum commands
		{
			stop,
			start,
			pwm,
			offset,
			cal,
			longTest,		//obsolete 
			autoTest,
			throttleTest,
			startTimer,
			stopTimer,
			pause,
			cont
		} stationCom=stop;
	}stationParameters;
	
	/*Structure of incoming/received data*/
	struct inParameters
	{
		int mins = 0, secs = 0, milis = 0, tenso = 0, rpm = 0, press1 = 0, press2 = 0;
		float timeInSecs = 0.0;
		float temp1, temp2, vibro[IN_VIBRO_COUNT], hal[IN_VIBRO_COUNT];
		float shunt = 0.0;
		float diffTime = 0.0;

		int oldTenso = 0, oldPres1=0, oldPres2=0;
		float  oldShunt=0.0, oldTemp1=0.0, oldTemp2=0.0;
	} chartValues;

	/*Charts initialization*/
	void setupCharts(void);

	void showValues(Thruster_Test::inParameters data);
	
	void setButtonsEnabled(bool connect, bool offset, bool cal, bool start, bool pwm, bool pause, bool stop, bool throt, bool autoTest);

	/*Frame functions*/
	void sendFrame(QByteArray out);
	QByteArray Thruster_Test::prepareFrame(Thruster_Test::outParameters ss);
	void Thruster_Test::partitionFrame(QByteArray inc, Thruster_Test::inParameters& data);
	
	Ui::Thruster_TestClass ui;

	QChart* pres1 = new QChart();
	QChart* pres2 = new QChart();
	QChart* temp1 = new QChart();
	QChart* temp2 = new QChart();
	QChart* curr = new QChart();
	QChart* hall = new QChart();
	QChart* vibr = new QChart();
	QChart* tens = new QChart();
	QLineSeries* temp1Series = new QLineSeries();
	QLineSeries* temp2Series = new QLineSeries();
	QLineSeries* pres1Series = new QLineSeries();
	QLineSeries* pres2Series = new QLineSeries();
	QLineSeries* currSeries = new QLineSeries();
	QLineSeries* hallSeries = new QLineSeries();
	QLineSeries* vibrSeries = new QLineSeries();
	QLineSeries* tensSeries = new QLineSeries();
	/*QValueAxis* xAxis = new QValueAxis;
	QValueAxis* t1Axis = new QValueAxis;
	QValueAxis* t2Axis = new QValueAxis;
	QValueAxis* p1Axis = new QValueAxis;
	QValueAxis* p2Axis = new QValueAxis;
	QValueAxis* p1Axis = new QValueAxis;
	QValueAxis* p2Axis = new QValueAxis;*/


	QTcpSocket  _socket;
	QByteArray outMessage;

	
	
};


/*MUST copy to ui_Thruster_Test after GUI recompilation from Qt Designer*/

/*
temp1->addSeries(temp1Series);
	tempChart1->setChart(temp1);


using namespace QtCharts;


	temp1Series.append(0, 10);
	temp1Series->append(10, 20);
	temp1Series->append(10, 30);
	temp1->addSeries(temp1Series);
	tempChart1->setChart(temp1);
*/