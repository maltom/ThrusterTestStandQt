#include "chartsSetup.h"
#include "ui_Thruster_Test.h"
#include <QtCharts/qchartglobal.h>
#include <QtCharts/QLineSeries>
#include "Thruster_Test.h"

using namespace QtCharts;

void chartsSetup(QChartView* tempChart1, QChartView* tempChart2, QChartView* currChart1, QChartView* currChart2, QChartView* presChart1, QChartView* presChart2, QChartView* vibrChart, QChartView* thruChart, QChartView* rpmsChart, QChartView* noisChart)
{
	QChart* pressure1 = new QChart();
	QChart* pressure2 = new QChart();
	QChart* temp1 = new QChart();
	QChart* temp2 = new QChart();
	QChart* current = new QChart();
	QLineSeries* temp1Series = new QLineSeries();
	temp1Series->append(0, 10);
	temp1Series->append(10, 20);
	temp1Series->append(10, 30);
	temp1->addSeries(temp1Series);
	tempChart1->setChart(temp1);
}
