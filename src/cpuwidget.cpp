#include "cpuwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>
#include <QSet>

static int coresNumberArray[] =
{
    1,2,4,6,8,
    10,12,14,16,18,
    20,22,24,26,28,
    30,32,34,36,38,
    40,42,44,46,48,
    50,52,54,56,58,
    60,62,64,
    128
};

static QString NumberStrinArray[] =
{
    "One",
    "Two",
    "Four",
    "Six",
    "Eight",

    "Ten",
    "Twelve",
    "Fourteen",
    "Sixteen",
    "Eighteen",

    "Twenty",
    "Twenty-two",
    "Twenty-four",
    "Twenty-six",
    "Twenty-eight",
    "Thirty",

    "Thirty-two",
    "Thirty-four",
    "Thirty-six",
    "Thirty-eight",
    "Fourty",

    "Fourty-two",
    "Fourty-four",
    "Fourty-six",
    "Fourty-eight",
    "Fifty",

    "Fifty-two",
    "Fifty-four",
    "Fifty-six",
    "Fifty-eight",
    "Sixty",

    "Sixty-two",
    "Sixty-four",
    "One hundred and Twenty-eight",
};

DWIDGET_USE_NAMESPACE

CpuWidget::CpuWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "CPU"))
{
    initWidget();
}

void CpuWidget::initWidget()
{ 
    QStringList cpuList = DeviceInfoParserInstance.getCpuList();

    QString architecture =  DeviceInfoParserInstance.queryData("lscpu", "lscpu", "Architecture");


    //setTitle(DApplication::translate("Main", "CPU")  + DApplication::translate("Main", " Info"));

    QStringList names = {   "Model",
                            "Architecture",
                            "Vendor",
                            DApplication::translate("CPU", "Speed"),
                            "CPU cores",
                            "Threads amount",
                            "L1d Cache",
                            "L1i Cache",
                            "L2 Cache",
                            "L3 Cache"
                        };

    overviewInfo_.value = DeviceInfoParserInstance.queryData("lscpu", "lscpu", "Model name");
    overviewInfo_.value.remove(" CPU", Qt::CaseInsensitive);

    QString maxSpeed = DeviceInfoParserInstance.queryData("lscpu", "lscpu", "CPU max MHz");
    QString minSpeed = DeviceInfoParserInstance.queryData("lscpu", "lscpu", "CPU min MHz");

    double maxMHz = maxSpeed.remove(",").toDouble()/1000.0;

    double minMHz = minSpeed.remove(",").toDouble()/1000.0;
    if(maxMHz > 10000)  //AMD CPU speed is MHZ, intel is GHZ
    {
        maxMHz/=10000.0;
        minMHz/=10000.0;
    }

    speed_.clear();

    int index = overviewInfo_.value.indexOf('@');
    if(index > 0)
    {
        speed_ = overviewInfo_.value.mid(index+1).trimmed();
    }
    else
    {
        speed_ = QString::number(maxMHz) + "GHz";
    }

    if(minMHz != maxMHz)
    {
        speed_ += " (";
        speed_ += QString::number(minMHz);
        speed_ += "GHz - ";
        speed_ += QString::number(maxMHz);
        speed_ += "GHz)";
    }

    if(cpuList.size() > 1)
    {
        QStringList headers = { "Name", "Core", /*"Vendor",*/ DApplication::translate("CPU", "Speed"), "Architecture" };
        QList<QStringList> tabList;

        foreach(const QString& cpu, cpuList)
        {
            QStringList tab = {
                DeviceInfoParserInstance.queryData("catcpu", cpu, "model name"),
                DApplication::translate("Main", "Core") +" " + cpu,
                //DeviceInfoParserInstance.queryData("catcpu", cpu, "vendor_id"),
                speed_,
                architecture
            };

            tabList.push_back(tab);
        }

        addTable(headers, tabList);
    }

    int cpus = DeviceInfoParserInstance.queryData("lscpu", "lscpu", "CPU(s)").toInt();
    int threadsPerCore = DeviceInfoParserInstance.queryData("lscpu", "lscpu", "Thread(s) per core").toInt();
    QString corePlus = " x " + QString::number(cpus);

    overviewInfo_.value += " ";
    int i = 0;
    for( ; i < sizeof(coresNumberArray)/sizeof(int); ++i)
    {
        if(coresNumberArray[i] == cpus)
        {
            break;
        }
    }

    if( i < sizeof(coresNumberArray)/sizeof(int) )
    {
        overviewInfo_.value += DApplication::translate("CPU", NumberStrinArray[i].toStdString().data() );
    }
    else
    {
        overviewInfo_.value += QString::number(cpus);
    }

    overviewInfo_.value += DApplication::translate("CPU", " Core(s)");

    QStringList contents = {
        overviewInfo_.value,
        architecture,
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "Vendor ID"),
        speed_,
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "CPU(s)"),
        QString::number(cpus*threadsPerCore),
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "L1d cache") + corePlus,
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "L1i cache") + corePlus,
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "L2 cache") + corePlus,
        DeviceInfoParserInstance.queryData("lscpu", "lscpu", "L3 cache"),
    };

    addInfo( "Cpu Info", names, contents);

    foreach(auto precessor, cpuList)
    {      
        addPrecessor(precessor);
    }
}

void CpuWidget::addPrecessor(const QString& precessor)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    ArticleStruct name("Name");
    name.queryData("catcpu", precessor, "model name" );
    articles.push_back(name);
    existArticles.insert("model name");

    ArticleStruct vendor("Vendor");
    vendor.queryData("catcpu", precessor, "vendor_id" );
    articles.push_back(vendor);
    existArticles.insert("vendor_id");

    ArticleStruct cpuid("Cpu id");
    cpuid.queryData("catcpu", precessor, "physical id" );
    articles.push_back(cpuid);
    existArticles.insert("physical id");

    ArticleStruct coreid("Core id");
    coreid.queryData("catcpu", precessor, "core id" );
    articles.push_back(coreid);
    existArticles.insert("core id");

    ArticleStruct threadamount("Threads amount");
    threadamount.queryData("lscpu", "lscpu", "Thread(s) per core" );
    articles.push_back(threadamount);

    ArticleStruct speed("Current Speed");
    speed.queryData("catcpu", precessor, "cpu MHz" );
    articles.push_back(speed);
    existArticles.insert("cpu MHz");

    ArticleStruct BogoMIPS("BogoMIPS");
    BogoMIPS.queryData( "catcpu", precessor, "bogomips" );
    articles.push_back(BogoMIPS);
    existArticles.insert("bogomips");

    ArticleStruct architecture("Architecture");
    architecture.queryData( "lscpu", "lscpu", "Architecture" );
    articles.push_back(architecture);

    ArticleStruct cpufamily("Cpu Family");
    cpufamily.queryData("catcpu", precessor, "cpu family" );
    articles.push_back(cpufamily);
    existArticles.insert("cpu family");

    ArticleStruct model("Model");
    model.queryData("catcpu", precessor, "model" );
    articles.push_back(model);
    existArticles.insert("model");

    ArticleStruct stepping("Stepping");
    stepping.queryData("catcpu", precessor, "stepping" );
    articles.push_back(stepping);
    existArticles.insert("stepping");

    ArticleStruct l1d( "L1d Cache");
    l1d.queryData( "lscpu", "lscpu", "L1d cache" );
    articles.push_back(l1d);

    ArticleStruct l1i( "L1i Cache");
    l1i.queryData( "lscpu", "lscpu", "L1i cache" );
    articles.push_back(l1i);

    ArticleStruct l2( "L2 Cache");
    l2.queryData( "lscpu", "lscpu", "L2 cache" );
    articles.push_back(l2);

    ArticleStruct l3( "L3 Cache");
    l3.queryData( "lscpu", "lscpu", "L3 cache" );
    articles.push_back(l3);

    ArticleStruct flags("Flags");
    flags.queryData( "catcpu", precessor, "flags" );
    articles.push_back(flags);
    existArticles.insert("flags");

    ArticleStruct virtualization("Virtualization");
    virtualization.queryData( "lscpu", "lscpu", "Virtualization" );
    articles.push_back(virtualization);

    DeviceInfoParserInstance.queryRemainderDeviceInfo("catcpu", precessor, articles, existArticles);

    addSubInfo( DApplication::translate("Main", "Core") + " " +  precessor, articles);
    articles.clear();
}
