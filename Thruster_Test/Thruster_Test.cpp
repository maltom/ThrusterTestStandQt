#include "Thruster_Test.h"
#include <QtWidgets/qlabel.h>
#include <QtCharts/qchartview.h>
#include <QtCharts/qlineseries.h>
#include <QtCharts/qcategoryaxis.h>
#include <qhostaddress.h>
#include <qmessagebox.h>
#include <fstream>
#include "qcustomplot.h"





#define IN_FRAME_SIZE 47
#define IN_VIBRO_COUNT 20



Thruster_Test::Thruster_Test(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	/*disabling the buttons at startup*/

	setButtonsEnabled(true, false, false, false, false, false, false, false, false);

	/*connection of slots and signals*/

	connect(ui.connect_button, SIGNAL(released()), this, SLOT(connectClick()));
	connect(ui.tenso_button, SIGNAL(released()), this, SLOT(tensoClick()));
	connect(ui.start_button, SIGNAL(released()), this, SLOT(startClick()));
	connect(ui.stop_button, SIGNAL(released()), this, SLOT(stopClick()));
	connect(ui.pause_button, SIGNAL(released()), this, SLOT(pauseClick()));
	connect(ui.offset_button, SIGNAL(released()), this, SLOT(offsetClick()));
	connect(ui.pwm_button, SIGNAL(released()), this, SLOT(pwmClick()));
	connect(ui.throttle_button, SIGNAL(released()), this, SLOT(throtClick()));
	connect(ui.auto_test_button, SIGNAL(released()), this, SLOT(autoTestClick()));

	connect(&_socket, SIGNAL(readyRead()), this, SLOT(frameRead()));
	connect(&_socket, SIGNAL(disconnect()), this, SLOT(conClosed()));

	connect(temp1Series, SIGNAL(pointAdded()), this, SLOT(refreshChart()));
	
	/*Chart setup*/
	setupCharts();
}

											/*BUTTON SLOTS*/

void Thruster_Test::connectClick(void)
{
	if (_socket.state() == QAbstractSocket::UnconnectedState)
	{
		_socket.connectToHost(QHostAddress(ui.ip_edit->text()), 80);
		if (!_socket.waitForConnected(1000))
		{
			QMessageBox::warning(this, tr("Connection error"),
				tr("TCP/IP connection failure.\n"
					"Check connection cable, power status or restart the STM controller."),
				QMessageBox::Ok);
			conClosed();
		}
		else
		{
			conOpened();
		}
	}
	else if (_socket.state() == QAbstractSocket::ConnectedState)
	{
		_socket.close();
		if (_socket.state() == QAbstractSocket::ClosingState) {
			
			connect(&_socket, SIGNAL(delayedCloseFinished()), this, SLOT(conClosed()));
		}
		else
		{
			conClosed();
		}
	}
}

void Thruster_Test::startClick(void)
{
	setButtonsEnabled(true, false, false, false, true, true, true, false, false);
	stationParameters.stationCom = Thruster_Test::outParameters::commands::start;
	
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
}
void Thruster_Test::stopClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::stop;

	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
	setButtonsEnabled(true, false, false, true, false, false, false, true, true);
	pres1->createDefaultAxes();
	pres2->createDefaultAxes();
	temp1->createDefaultAxes();
	temp2->createDefaultAxes();
	curr->createDefaultAxes();
	hall->createDefaultAxes();
	vibr->createDefaultAxes();
	tens->createDefaultAxes();

	pres1->removeSeries(pres1Series);
	pres2->removeSeries(pres2Series);
	temp1->removeSeries(temp1Series);
	temp2->removeSeries(temp2Series);
	curr->removeSeries(currSeries);
	hall->removeSeries(hallSeries);
	vibr->removeSeries(vibrSeries);
	tens->removeSeries(tensSeries);

	pres1->addSeries(pres1Series);
	pres2->addSeries(pres2Series);
	temp1->addSeries(temp1Series);
	temp2->addSeries(temp2Series);
	curr->addSeries(currSeries);
	hall->addSeries(hallSeries);
	vibr->addSeries(vibrSeries);
	tens->addSeries(tensSeries);
}
void Thruster_Test::pwmClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::pwm;
	stationParameters.offsetDuty = ui.pwm_spin->value();
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
	
}
void Thruster_Test::pauseClick(void)
{
	if (!pauseState)
	{
		ui.pause_button->setText("Continue");
		stationParameters.stationCom = Thruster_Test::outParameters::commands::pause;
		pauseState = 1;
	}
	else
	{
		ui.pause_button->setText("Pause");
		stationParameters.stationCom = Thruster_Test::outParameters::commands::cont;
		pauseState = 0;
	}
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
}

void Thruster_Test::tensoClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::cal;
	stationParameters.offsetDuty = ui.offset_edit->text().toInt();
	setButtonsEnabled(true, false, false, true, false, false, false, true, true);
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
	QPalette pal;
	pal.setColor(QPalette::Base, Qt::darkGreen);
	ui.callibration_status_edit->setPalette(pal);
}
void Thruster_Test::offsetClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::offset;
	setButtonsEnabled(true, false, true, false, false, false, false, false, false);
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
}
void Thruster_Test::throtClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::throttleTest;
	setButtonsEnabled(true, false, false, false, false, false, false, false, false);
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
}
void Thruster_Test::autoTestClick(void)
{
	stationParameters.stationCom = Thruster_Test::outParameters::commands::autoTest;
	stationParameters.riseTime = ui.rise_time_edit->text().toInt();
	stationParameters.stayTime = ui.stay_time_edit->text().toInt();
	stationParameters.fallTime = ui.fall_time_edit->text().toInt();
	stationParameters.offsetDuty = ui.pwm_spin->value();
	setButtonsEnabled(true, false, false, false, false, false, false, false, false);
	outMessage = prepareFrame(stationParameters);
	sendFrame(outMessage);
}

											/*TCP SLOTS*/
void Thruster_Test::frameRead(void)
{
	QByteArray datas = _socket.readAll();
	std::ofstream ramk("paruffka.txt", std::ios::app);
	if (ramk.good())
		ramk << datas.toStdString();
	ramk.close();
	partitionFrame(datas, chartValues);

	//ui.statusBar->showMessage(datas);

	
}
void Thruster_Test::conOpened(void)
{
	ui.statusBar->showMessage("Connected!", 5000);
	QPalette pal;
	pal.setColor(QPalette::Base, Qt::darkGreen);
	ui.connect_status_edit->setPalette(pal);
	ui.connect_button->setText("Disconnect");
	setButtonsEnabled(true, true, false, false, false, false, false, false, false);

}
void Thruster_Test::conClosed(void)
{
	QPalette pal;
	pal.setColor(QPalette::Base, Qt::darkRed);
	ui.connect_status_edit->setPalette(pal);
	ui.statusBar->showMessage("Disconnected!", 5000);
	ui.connect_button->setText("Connect");
	setButtonsEnabled(true, false, false, false, false, false, false, false, false);
	ui.callibration_status_edit->setPalette(pal);
}
void Thruster_Test::setButtonsEnabled(bool connect, bool offset, bool cal, bool start, bool pwm, bool pause, bool stop, bool throt, bool autoTest)
{
	ui.offset_button->setEnabled(offset);
	ui.tenso_button->setEnabled(cal);
	ui.pwm_button->setEnabled(pwm);
	ui.start_button->setEnabled(start);
	ui.stop_button->setEnabled(stop);
	ui.pause_button->setEnabled(pause);
	ui.auto_test_button->setEnabled(autoTest);
	ui.throttle_button->setEnabled(throt);
	
}
										/*Frame maniupaltion*/

QByteArray Thruster_Test::prepareFrame(Thruster_Test::outParameters ss)
{
	QByteArray out;
	out.append("[");
	out.append(QByteArray::number(ss.stationCom));
	out.append("|");
	out.append(QByteArray::number(ss.offsetDuty));
	out.append("|");
	out.append(QByteArray::number(ss.riseTime));
	out.append("|");
	out.append(QByteArray::number(ss.stayTime));
	out.append("|");
	out.append(QByteArray::number(ss.fallTime));
	out.append("]");

	return out;
}
void Thruster_Test::partitionFrame(QByteArray inc, Thruster_Test::inParameters& data)
{
	std::vector<std::string> singleVals;
	std::string window = "";

	int frameindex = 0;
	int corrupted = 0;
	//frame solution for verification 
	
	while (frameindex < inc.size())
	{
		if (inc[frameindex] == ']')
		{
			singleVals.push_back(window);
			break;
		}
		else
		{
			if (inc[frameindex] != '[')
			{
				if (inc[frameindex] != '|' && inc[frameindex] != ',' && inc[frameindex] != ':')
					window += inc[frameindex];
				else
				{
					if (inc[frameindex + 1] != '|' && inc[frameindex + 1] != ']')
					{
						singleVals.push_back(window);
						window = "";
					}
					else
					{
						corrupted = 1;
						break;
					}
				}
			}
			frameindex++;
		}
	}
	if (singleVals.size() != 47)
		corrupted = 1;

	std::vector<float> numberVals;
	if (!corrupted)
	{
		for (auto word : singleVals)
		{
			numberVals.push_back(std::stof(word));
		}
		if (numberVals.size() > 4)
		{
			data.mins = (int)numberVals[0];
			data.secs = (int)numberVals[1];
			data.milis = (int)numberVals[2];
			data.oldTemp1 = data.temp1;
			data.oldTemp2 = data.temp2;
			data.temp1 = numberVals[3];
			data.temp2 = numberVals[4];

			int i = 5;
			int j = i + IN_VIBRO_COUNT;
			for (; i < j; ++i)
			{
				data.vibro[i - 5] = (int)numberVals[i];
			}
			
			data.oldShunt = data.shunt;
			data.shunt = numberVals[i];

			j = i + IN_VIBRO_COUNT;
			int k = i;
			for (; i < j && i < numberVals.size() - 1; ++i)
			{
				data.hal[i - k] = (int)numberVals[i];
			}
			++i;
			data.oldTenso = data.tenso;
			data.tenso = (int)numberVals[i];

			data.diffTime = (data.secs + 0.001 * data.milis + 60 * data.mins - data.timeInSecs) / (float)IN_VIBRO_COUNT;
			data.timeInSecs = data.secs + 0.001 * data.milis + 60 * data.mins;
			
		}
		showValues(chartValues);
	}
	else
	{
		ui.statusBar->showMessage("Corrupted Frame(s) occured!", 3000);
	}
	
}

									/*Frame Sending*/
void Thruster_Test::sendFrame(QByteArray out)
{
	_socket.write(out);
}

void Thruster_Test::showValues(Thruster_Test::inParameters data)
{
	ui.temperature_val1->setText(QString::number(data.temp1));
	ui.temperature_val2->setText(QString::number(data.temp2));
	ui.pressure_val1->setText(QString::number(data.press1));
	ui.pressure_val2->setText(QString::number(data.press2));
	ui.current_val1->setText(QString::number(data.shunt));
	ui.current_val2->setText(QString::number(data.hal[IN_VIBRO_COUNT-1]));
	ui.vibrations_val->setText(QString::number(data.vibro[IN_VIBRO_COUNT]));
	ui.thrust_val->setText(QString::number(data.tenso));
	
		pres1Series->append(data.timeInSecs, data.oldPres1);
		pres1Series->append(data.timeInSecs, data.press1);
		pres2Series->append(data.timeInSecs, data.oldPres2);
		pres2Series->append(data.timeInSecs, data.press2);

		temp1Series->append(data.timeInSecs, data.oldTemp1);
		temp1Series->append(data.timeInSecs, data.temp1);
		temp1Series->append(data.timeInSecs, data.oldTemp2);
		temp2Series->append(data.timeInSecs, data.temp2);
		currSeries->append(data.timeInSecs, data.oldShunt);
		currSeries->append(data.timeInSecs, data.shunt);
		tensSeries->append(data.timeInSecs, data.oldTenso);
		tensSeries->append(data.timeInSecs, data.tenso);
	
	for (int i = 0; i < IN_VIBRO_COUNT; ++i)
	{
		hallSeries->append(data.timeInSecs - (((float)IN_VIBRO_COUNT - 1 - ((float)i)) * data.diffTime), data.hal[i]);
		vibrSeries->append(data.timeInSecs - (((float)IN_VIBRO_COUNT - 1 - ((float)i)) * data.diffTime), data.vibro[i]);
	}
	
	
	

	/*pres1->createDefaultAxes();
	pres2->createDefaultAxes();
	temp1->createDefaultAxes();
	temp2->createDefaultAxes();
	curr->createDefaultAxes();
	hall->createDefaultAxes();
	vibr->createDefaultAxes();
	tens->createDefaultAxes();
	/*pres1->update();
	pres2->update();
	temp1->update();
	temp2->update();
	curr->update();
	hall->update();
	vibr->update();
	tens->update();
	ui.pressure_graph1->update();
	ui.pressure_graph2->update();
	ui.temperature_graph1->update();
	ui.temperature_graph2->update();
	ui.current_graph1->update();
	ui.current_graph2->update();
	ui.vibrations_graph->update();
	ui.thrust_graph->update();*/
	//setupCharts();

}

void Thruster_Test::setupCharts(void)
{
	pres1->addSeries(pres1Series);
	pres2->addSeries(pres2Series);
	temp1->addSeries(temp1Series);
	temp2->addSeries(temp2Series);
	curr->addSeries(currSeries);
	hall->addSeries(hallSeries);
	vibr->addSeries(vibrSeries);
	tens->addSeries(tensSeries);

	pres1->createDefaultAxes();
	pres2->createDefaultAxes();
	temp1->createDefaultAxes();
	temp2->createDefaultAxes();
	curr->createDefaultAxes();
	hall->createDefaultAxes();
	vibr->createDefaultAxes();
	tens->createDefaultAxes();

	pres1->legend()->hide();
	pres2->legend()->hide();
	temp1->legend()->hide();
	temp2->legend()->hide();
	curr->legend()->hide();
	hall->legend()->hide();
	vibr->legend()->hide();
	tens->legend()->hide();
	
	/*
	temp1->setAxisX(xAxis);
	temp1->setAxisY(t1Axis);
	xAxis->setMax(chartValues.timeInSecs);
	xAxis->setMin(0);
	t1Axis->setMax(50);
	t1Axis->set*/

	ui.pressure_graph1->setChart(pres1);
	ui.pressure_graph2->setChart(pres2);
	ui.temperature_graph1->setChart(temp1);
	ui.temperature_graph2->setChart(temp2);
	ui.current_graph1->setChart(curr);
	ui.current_graph2->setChart(hall);
	ui.vibrations_graph->setChart(vibr);
	ui.thrust_graph->setChart(tens);


	ui.pressure_graph1->setRenderHint(QPainter::Antialiasing);
	ui.pressure_graph2->setRenderHint(QPainter::Antialiasing);
	ui.temperature_graph1->setRenderHint(QPainter::Antialiasing);
	ui.temperature_graph2->setRenderHint(QPainter::Antialiasing);
	ui.current_graph1->setRenderHint(QPainter::Antialiasing);
	ui.current_graph2->setRenderHint(QPainter::Antialiasing);
	ui.vibrations_graph->setRenderHint(QPainter::Antialiasing);
	ui.thrust_graph->setRenderHint(QPainter::Antialiasing);

	ui.pressure_graph1->setRubberBand(QChartView::HorizontalRubberBand);
	ui.pressure_graph2->setRubberBand(QChartView::HorizontalRubberBand);
	ui.temperature_graph1->setRubberBand(QChartView::HorizontalRubberBand);
	ui.temperature_graph2->setRubberBand(QChartView::HorizontalRubberBand);
	ui.current_graph1->setRubberBand(QChartView::HorizontalRubberBand);
	ui.current_graph2->setRubberBand(QChartView::HorizontalRubberBand);
	ui.vibrations_graph->setRubberBand(QChartView::HorizontalRubberBand);
	ui.thrust_graph->setRubberBand(QChartView::HorizontalRubberBand);
}

void Thruster_Test::refreshChart(void)
{
	temp1->removeSeries(temp1Series);
	temp1->addSeries(temp1Series);

}