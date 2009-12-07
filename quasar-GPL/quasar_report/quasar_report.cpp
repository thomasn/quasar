#include <qapplication.h>
#include <qsocket.h>

#include "report_defn.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "query_data_source.h"
#include "data_source_factory.h"
#include "lookup_param_type.h"
#include "param_type_factory.h"
#include "company.h"
#include "store.h"

extern void quasarTclInit(ReportInterp* i, void* arg);

int
main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Report to print
    QString reportName;

    // Database connection
    QString hostname;
    int port = 3599;
    QString companyName;
    QString username;
    QString password;

    // Parameter values
    QMap<QString,QString> paramValues;

    // Output choice
    QString printerName;

    // Just list param names and exit?
    bool listParams = false;

    // Process command line args
    for (int i = 1; i < app.argc(); ++i) {
	QString arg = app.argv()[i];

	if (arg == "-report") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -report argument");
	    reportName = app.argv()[++i];
	} else if (arg == "-server") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -server argument");
	    hostname = app.argv()[++i];
	    // TODO: check server for hostname:port method
	} else if (arg == "-company") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -company argument");
	    companyName = app.argv()[++i];
	} else if (arg == "-username") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -username argument");
	    username = app.argv()[++i];
	} else if (arg == "-password") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -password argument");
	    password = app.argv()[++i];
	} else if (arg == "-printer") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -printer argument");
	    printerName = app.argv()[++i];
	} else if (arg == "-param") {
	    if (i == app.argc() - 1)
		qFatal("Error: missing -param argument");
	    QString name = app.argv()[++i];
	    if (i == app.argc() - 1 || QString(app.argv()[++i]) != "-value")
		qFatal("Error: missing -value after -param");
	    if (i == app.argc() - 1)
		qFatal("Error: missing -value argument");
	    paramValues[name] = app.argv()[++i];
	} else if (arg == "-listParams") {
	    listParams = true;
	} else if (arg == "-help") {
	    qDebug("Usage: quasar_report -report <file> [-printer <name>]");
	    return 0;
	} else {
	    qFatal("Error: unknown argument: " + arg);
	}
    }

    // Validate command line args
    if (reportName.isEmpty())
	qFatal("Error: you must specify a report file");
    if (hostname.isEmpty())
	qFatal("Error: you must specify a server");
    if (companyName.isEmpty())
	qFatal("Error: you must specify a company");
    if (username.isEmpty())
	qFatal("Error: you must specify a username");

    // Connect to server
    QuasarClient client;
    client.serverConnect(hostname, port);
    while (!client.serverConnected()) {
	qApp->processEvents();
	if (client.serverState() == QSocket::Idle)
	    qFatal("Error: failed connecting to " + hostname);
    }

    // Open company
    if (!client.openCompany(companyName, username, password))
	qFatal("Error: failed connecting to company");

    Company company;
    client.db()->lookup(company);

    // Get default store
    Id store_id = client.defaultStore();
    Store store;
    if (store_id == INVALID_ID || !client.db()->lookup(store_id, store)) {
	store_id = client.user().defaultStore();
	if (store_id == INVALID_ID)
	    store_id = company.defaultStore();
    }

    // Register query data source
    if (!DataSourceFactory::typeExists("query"))
	DataSourceFactory::registerType("query", createQueryDataSource);
    QueryDataSource::connection = client.connection();

    // Register lookup param type
    if (!ParamTypeFactory::typeExists("lookup"))
	ParamTypeFactory::registerType("lookup", createLookupParamType);
    LookupParamType::quasar = &client;

    // Set internal variables
    ReportParamType::setInternal("current_company", company.id().toString());
    ReportParamType::setInternal("current_store", store_id.toString());
    ReportParamType::setInternal("current_station",
				 client.defaultStation().toString());
    ReportParamType::setInternal("current_employee",
				 client.defaultEmployee().toString());

    // Register callback on Tcl interp init
    ReportInterp::addCallback("tcl", quasarTclInit, client.connection());

    // Fetch report
    QString reportFile;
    if (!client.resourceFetch("reports", reportName, reportFile))
	qFatal("Error: failed fetching report: " + reportName);

    // Load report definition
    ReportDefn report;
    if (!report.load(reportFile))
	qFatal("Error: failed loading report: " + reportName);

    // List params and exit if requested
    if (listParams) {
	qDebug("Parameters for " + reportName + ":");
	for (unsigned int i = 0; i < report.parameters.size(); ++i) {
	    const ReportParam& param = report.parameters[i];

	    qDebug("    " + param.name + ": " + param.type);
	}
	return 0;
    }

    // Process parameters
    ParamMap params;
    for (unsigned int i = 0; i < report.parameters.size(); ++i) {
	const ReportParam& param = report.parameters[i];
	ReportParamType* type = ParamTypeFactory::create(param.type);
	if (type == NULL)
	    qFatal("Invalid parameter type: " + param.type);
	type->setParam(param);

	QString text;
	if (paramValues.contains(param.name))
	    text = paramValues[param.name];
	else if (!param.defaultValue.isEmpty())
	    text = param.defaultValue;

	if (text.isEmpty() && param.manditory)
	    qFatal("Param " + param.name + " is manditory");

	QVariant value;
	type->convert(text, value);
	params[param.name] = value;
    }

    // Generate and print
    ReportOutput output;
    if (!report.generate(params, output))
	qFatal("Error: failed generating report: " + reportName);
    output.print(NULL, printerName, false);

    return 0;
}
