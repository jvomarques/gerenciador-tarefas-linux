#include "proc.cpp"
#include <unistd.h>

using namespace std;

#define BUF_MAX 1024
#define MAX_CPU 128

int read_fields (FILE *fp, unsigned long long int *fields)
{
  int retval;
  char buffer[BUF_MAX];


  if (!fgets (buffer, BUF_MAX, fp))
  { perror ("Error"); }
  /* line starts with c and a string. This is to handle cpu, cpu[0-9]+ */
  retval = sscanf (buffer, "c%*s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
                            &fields[0],
                            &fields[1],
                            &fields[2],
                            &fields[3],
                            &fields[4],
                            &fields[5],
                            &fields[6],
                            &fields[7],
                            &fields[8],
                            &fields[9]);
  if (retval == 0)
  { return -1; }
  if (retval < 4) /* Atleast 4 fields is to be read */
  {
    fprintf (stderr, "Error reading /proc/stat cpu field\n");
    return 0;
  }
  return 1;
}

vector<double> usageCPU()
{
  FILE *fp;
  vector<double> ret;

  unsigned long long int fields[10], total_tick[MAX_CPU], total_tick_old[MAX_CPU], idle[MAX_CPU], idle_old[MAX_CPU], del_total_tick[MAX_CPU],   del_idle[MAX_CPU];
  int update_cycle = 0, i, cpus = 0, count;
  double percent_usage;


  fp = fopen ("/proc/stat", "r");
  if (fp == NULL)
  {
    perror ("Error");
  }


  while (read_fields (fp, fields) != -1)
  {
    for (i=0, total_tick[cpus] = 0; i<10; i++)
    { total_tick[cpus] += fields[i]; }
    idle[cpus] = fields[3]; /* idle ticks index */
    cpus++;
  }
  usleep (200000);
  fseek (fp, 0, SEEK_SET);
  fflush (fp);
  for (count = 0; count < cpus; count++)
  {
    total_tick_old[count] = total_tick[count];
    idle_old[count] = idle[count];

    if (!read_fields (fp, fields))
    { vector<double> r;return r; }

    for (i=0, total_tick[count] = 0; i<10; i++)
    { total_tick[count] += fields[i]; }
    idle[count] = fields[3];

    del_total_tick[count] = total_tick[count] - total_tick_old[count];
    del_idle[count] = idle[count] - idle_old[count];

    percent_usage = ((del_total_tick[count] - del_idle[count]) / (double) del_total_tick[count]) * 100;
    if (count != 0){
        ret.push_back(percent_usage);
    }

      update_cycle++;
      printf ("\n");
  }
  fclose(fp);
    return ret;
}

void delEspacos(string &str)
{
   str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

double usoRAM(){

    system("egrep  'MemTotal|MemFree|SwapTotal|SwapFree' /proc/meminfo > ram.txt");
    FILE *ram;

    char memtotal[1000000];
    char aux[1000000];

    char memlivre[1000000];

    ram = fopen("ram.txt", "r");

    fscanf(ram, "%s %s %s\n" , aux, memtotal, aux);
    fscanf(ram, "%s %s\n", aux, memlivre);

    fclose(ram);

    string str(memtotal);
    string str2(memlivre);
    delEspacos(str);
    delEspacos(str2);

    double ramTotal = atoi(str.c_str());;
    double ramLivre = atoi(str2.c_str());;
    double ramUso_MB = (ramTotal - ramLivre)/1000000;

    return ramUso_MB;
}

double usoSwap(){

    system("egrep  'SwapTotal|SwapFree' /proc/meminfo > swap.txt");
    FILE *swap;

    char memtotal[1000000];
    char aux[1000000];

    char memlivre[1000000];

    swap = fopen("swap.txt", "r");

    fscanf(swap, "%s %s %s\n" , aux, memtotal, aux);
    fscanf(swap, "%s %s\n", aux, memlivre);

    fclose(swap);

    string str(memtotal);
    string str2(memlivre);
    delEspacos(str);
    delEspacos(str2);

    double swapTotal = atoi(str.c_str());;
    double swapLivre = atoi(str2.c_str());;
    double swapUso_MB = (swapTotal - swapLivre)/1000000;

    return swapUso_MB;
}

int cargaAtual(){

    system("cat /sys/class/power_supply/BAT1/uevent | grep  POWER_SUPPLY_CAPACITY= > carga.txt");
    FILE *carga;

    carga = fopen("carga.txt", "r");

    char a[1000000];

    fscanf(carga, "%s\n", a);

    string str(a);
    string cargaString = str.substr(22,24);
    int cargaAtual = atoi(cargaString.c_str());

    //cout << cargaAtual << endl;
    fclose(carga);
    return cargaAtual;
}

double tempoRestante(){

    //PRECISA INSTALAR O 'acpi'.
    system("acpi > tempo_restante.txt");
    FILE *tempo;

    tempo = fopen("tempo_restante.txt", "r");

    char aux[1000000];
    char a[1000000];

    fscanf(tempo, "%s %s %s %s %s\n", aux, aux, aux, aux, a);

    string tempoString(a);

    string hora = tempoString.substr(1,1);
    string minutos = tempoString.substr(3,2);;
    string segundos;

    double horaInt = atoi(hora.c_str());
    double MinutosInt = atoi(minutos.c_str());
    double tempoRestanteFinal = horaInt*60+MinutosInt;

    fclose(tempo);
    return tempoRestanteFinal;

}

bool ehPid(int k){

    string kString = IntToString(k);
    string pasta = "/proc/";
    pasta = strcat(&pasta[0], &kString[0]);
    DIR* dir = opendir(&pasta[0]);
    if (dir)
    {
        closedir(dir);
        return true;
    }
    return false;
}

vector< Proc > processos;

QFile arquivoMain("processos.json");
QTextStream escreve(&arquivoMain);

void excluirProcessos(){
    vector< Proc > aux;
    processos = aux;
}

void inicializarProcessos(){
    Proc pro_aux; pro_aux.setPid(-1);
    for (int i=0; i < 1000000; i++)
        processos.push_back(pro_aux);
}

void setarProcessos(){
    for (int i = 0; i<100000; ++i)
    {
        if (ehPid(i))
        {
            Proc p; p.setPid(i);
            processos[i] = p;
        }
    }
}

void setarFilhos(){
    for (int i = 0; i<100000; ++i)
        if (processos[i].pid != -1)
            processos[processos[i].ppid].filhos.push_back(processos[i]);
}

void initProcessos(){
    excluirProcessos();
    inicializarProcessos();
    setarProcessos();
    setarFilhos();
}

void filtroCPU(int pd,int tam,bool flag){
    if(processos[pd].filhos.size()==0){
        if(flag) escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].cpu << ", \"pid\": " << pd << " },";
        else escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].cpu << ", \"pid\": " << pd << " }";
    }else{
        escreve << "{ \"name\" : \"" << processos[pd].nome << "\",\n" "\"children\":\n [\n";
        for (int i=0; i < processos[pd].filhos.size();++i)
            filtroCPU(processos[pd].filhos[i].pid, ++tam,!(i==(processos[pd].filhos.size()-1)));
        escreve << "]\n }";
        if(flag) escreve <<",\n";
        else escreve << "\n";
    }
}
void filtroThread(int pd,int tam,bool flag){
    if(processos[pd].filhos.size()==0){
        if(flag) escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].thread << ", \"pid\": " << pd << " },";
        else escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].thread << ", \"pid\": " << pd << " }";
    }else{
        escreve << "{ \"name\" : \"" << processos[pd].nome << "\",\n" "\"children\":\n [\n";
        for (int i=0; i < processos[pd].filhos.size();++i)
            filtroThread(processos[pd].filhos[i].pid, ++tam,!(i==(processos[pd].filhos.size()-1)));
        escreve << "]\n }";
        if(flag) escreve <<",\n";
        else escreve << "\n";
    }
}
void filtroMem(int pd,int tam,bool flag){
    if(processos[pd].filhos.size()==0){
        if(flag) escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].mem << ", \"pid\": " << pd << " },";
        else escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].mem << ", \"pid\": " << pd << " }";
    }else{
        escreve << "{ \"name\" : \"" << processos[pd].nome << "\",\n" "\"children\":\n [\n";
        for (int i=0; i < processos[pd].filhos.size();++i)
            filtroMem(processos[pd].filhos[i].pid, ++tam,!(i==(processos[pd].filhos.size()-1)));
        escreve << "]\n }";
        if(flag) escreve <<",\n";
        else escreve << "\n";
    }
}
void filtroGourmet(int pd,int tam,bool flag){
    if(processos[pd].filhos.size()==0){
        if(flag) escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].gourmet << ", \"pid\": " << pd << " },";
        else escreve << "{ \"name\" : \"" << processos[pd].nome << "\", \"size\": " << processos[pd].gourmet << ", \"pid\": " << pd << " }";
    }else{
        escreve << "{ \"name\" : \"" << processos[pd].nome << "\",\n" "\"children\":\n [\n";
        for (int i=0; i < processos[pd].filhos.size();++i)
            filtroGourmet(processos[pd].filhos[i].pid, ++tam,!(i==(processos[pd].filhos.size()-1)));
        escreve << "]\n }";
        if(flag) escreve <<",\n";
        else escreve << "\n";
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile x("processos.json");
    x.open(QFile::WriteOnly | QFile::Text);
    QTextStream xout(&x);
    xout << " ";
    x.flush();
    x.close();

    initProcessos();

    arquivoMain.open(QFile::WriteOnly | QFile::Text | QIODevice::Append);
    filtroCPU(0,0,0);
    arquivoMain.flush();
    arquivoMain.close();
    ui->webViewProcessos->load(QUrl("file:///" + QCoreApplication::applicationDirPath()+ "/index.html"));
    inicializarGraficoCPU();
    inicializarGraficoMemoria();
    inicializarGraficoCarga();
    inicializarGraficoTempoDescarga();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::inicializarGraficoCPU()
{
    //Adicionar CPU 1 - Azul
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue));

    //Adicionar CPU 2 - Vermelha
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red));

    //Adicionar CPU 3 - Verde
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::green));

    //Adicionar CPU 4 - Amarelo
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setPen(QPen(Qt::yellow));

    //Configuração do eixo X
    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot->xAxis->setTickLabelRotation(90);
    ui->customPlot->xAxis->setAutoTickStep(false);
    ui->customPlot->xAxis->setTickStep(2);
    ui->customPlot->axisRect()->setupFullAxesBox();

    //Configuração da Legenda
    ui->customPlot->graph(0)->setName("CPU 1");
    ui->customPlot->graph(1)->setName("CPU 2");
    ui->customPlot->graph(2)->setName("CPU 3");
    ui->customPlot->graph(3)->setName("CPU 4");
    ui->customPlot->legend->setVisible(true);

    ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
    ui->customPlot->legend->setFont(QFont("Helvetica",9));
    ui->customPlot->legend->setRowSpacing(-10);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(atualizarGraficoCPU()));
    dataTimer.start(300); // Interval 0 means to refresh as fast as possible
}

void MainWindow::inicializarGraficoMemoria()
{
    //Adicionar Memória Principal - Azul
    ui->customPlot2->addGraph();
    ui->customPlot2->graph(0)->setPen(QPen(Qt::blue));

    //Adicionar Swap - Vermelha
    ui->customPlot2->addGraph();
    ui->customPlot2->graph(1)->setPen(QPen(Qt::red));

    //Adicionar circulo azul seguidor da memória principal
    ui->customPlot2->addGraph(); // blue dot
    ui->customPlot2->graph(2)->setPen(QPen(Qt::blue));
    ui->customPlot2->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot2->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);

    //Adicionar circulo vermelho seguidor do swap
    ui->customPlot2->addGraph(); // red dot
    ui->customPlot2->graph(3)->setPen(QPen(Qt::red));
    ui->customPlot2->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot2->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    //Configuração do eixo X
    ui->customPlot2->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot2->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot2->xAxis->setTickLabelRotation(90);
    ui->customPlot2->xAxis->setAutoTickStep(false);
    ui->customPlot2->xAxis->setTickStep(2);
    ui->customPlot2->axisRect()->setupFullAxesBox();

    //Configuração da Legenda
    ui->customPlot2->graph(0)->setName("RAM (GB)");
    ui->customPlot2->graph(1)->setName("Swap (GB)");
    ui->customPlot2->legend->setVisible(true);
    ui->customPlot2->legend->removeItem(3); //remover a legenda do circulo seguidor
    ui->customPlot2->legend->removeItem(2); //remover a legenda do circulo seguidor
    ui->customPlot2->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
    ui->customPlot2->legend->setFont(QFont("Helvetica",9));
    ui->customPlot2->legend->setRowSpacing(-10);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot2->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&memoriaTimer, SIGNAL(timeout()), this, SLOT(atualizarGraficoMemoria()));
    memoriaTimer.start(0); // Interval 0 means to refresh as fast as possible
}


void MainWindow::inicializarGraficoCarga()
{
    //Adicionar Carga - Azul
    ui->customPlot3->addGraph();
    ui->customPlot3->graph(0)->setPen(QPen(Qt::blue));

    //Adicionar circulo azul seguidor da Carga
    ui->customPlot3->addGraph(); // blue dot
    ui->customPlot3->graph(1)->setPen(QPen(Qt::blue));
    ui->customPlot3->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot3->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);

    //Configuração do eixo X
    ui->customPlot3->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot3->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot3->xAxis->setTickLabelRotation(90);
    ui->customPlot3->xAxis->setAutoTickStep(false);
    ui->customPlot3->xAxis->setTickStep(2);
    ui->customPlot3->axisRect()->setupFullAxesBox();

    //Configuração da Legenda
    ui->customPlot3->graph(0)->setName("Carga (%)");
    ui->customPlot3->legend->setVisible(true);
    ui->customPlot3->legend->removeItem(1); //remover a legenda do circulo seguidor
    ui->customPlot3->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
    ui->customPlot3->legend->setFont(QFont("Helvetica",9));
    ui->customPlot3->legend->setRowSpacing(-10);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot3->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot3->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot3->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot3->yAxis2, SLOT(setRange(QCPRange)));

    //MODIFICAR ESSA FUNCIONALIDADE PELA EXECUÇÃO DE UMA THREAD

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&cargaTimer, SIGNAL(timeout()), this, SLOT(atualizarGraficoCarga()));
    cargaTimer.start(0); // Interval 0 means to refresh as fast as possible
}


void MainWindow::inicializarGraficoTempoDescarga()
{
    //Adicionar Tempo Descarga - Vermelho
    ui->customPlot4->addGraph();
    ui->customPlot4->graph(0)->setPen(QPen(Qt::red));

    //Adicionar circulo vermelho seguidor da Carga
    ui->customPlot4->addGraph(); // red dot
    ui->customPlot4->graph(1)->setPen(QPen(Qt::red));
    ui->customPlot4->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot4->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);

    //Configuração do eixo X
    ui->customPlot4->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot4->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot4->xAxis->setTickLabelRotation(90);
    ui->customPlot4->xAxis->setAutoTickStep(false);
    ui->customPlot4->xAxis->setTickStep(2);
    ui->customPlot4->axisRect()->setupFullAxesBox();

    //Configuração da Legenda
    ui->customPlot4->graph(0)->setName("Tempo de Descarga (min)");
    ui->customPlot4->legend->setVisible(true);
    ui->customPlot4->legend->removeItem(1); //remover a legenda do circulo seguidor
    ui->customPlot4->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);
    ui->customPlot4->legend->setFont(QFont("Helvetica",9));
    ui->customPlot4->legend->setRowSpacing(-10);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot4->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot4->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot4->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot4->yAxis2, SLOT(setRange(QCPRange)));

    //MODIFICAR ESSA FUNCIONALIDADE PELA EXECUÇÃO DE UMA THREAD

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&tempoDescargaTimer, SIGNAL(timeout()), this, SLOT(atualizarGraficoTempoDescarga()));
    tempoDescargaTimer.start(1000); // Interval 0 means to refresh as fast as possible
}

void MainWindow::atualizarGraficoCPU()
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    vector<double> vet = usageCPU();
    double value0 = vet[0];
    double value1 = vet[1];
    double value2 = vet[2];
    double value3 = vet[3];

    // add data to lines:
      ui->customPlot->graph(0)->addData(key, value0);
      ui->customPlot->graph(1)->addData(key, value1);
      // add data to lines:
        ui->customPlot->graph(2)->addData(key, value2);
        ui->customPlot->graph(3)->addData(key, value3);



      ui->customPlot->graph(0)->rescaleValueAxis();
      ui->customPlot->graph(1)->rescaleValueAxis(true);
      ui->customPlot->graph(2)->rescaleValueAxis();
      ui->customPlot->graph(3)->rescaleValueAxis(true);

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key+0.25,60, Qt::AlignRight);
    ui->customPlot->replot();

}

void MainWindow::atualizarGraficoMemoria()
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    double value0 = usoRAM();
    double value1 = usoSwap();

    // add data to lines:
      ui->customPlot2->graph(0)->addData(key, value0);
      ui->customPlot2->graph(1)->addData(key, value1);

      // set data of dots:
      ui->customPlot2->graph(2)->clearData();
      ui->customPlot2->graph(2)->addData(key, value0);
      ui->customPlot2->graph(3)->clearData();
      ui->customPlot2->graph(3)->addData(key, value1);

      ui->customPlot2->graph(0)->rescaleValueAxis();
      ui->customPlot2->graph(1)->rescaleValueAxis(true);

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot2->xAxis->setRange(key+0.25,60, Qt::AlignRight);
    ui->customPlot2->replot();

}

void MainWindow::atualizarGraficoCarga()
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    double value0 = cargaAtual();

    // add data to lines:
    ui->customPlot3->graph(0)->addData(key, value0);

    // set data of dots:
    ui->customPlot3->graph(1)->clearData();
    ui->customPlot3->graph(1)->addData(key, value0);

    ui->customPlot3->graph(0)->rescaleValueAxis();

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot3->xAxis->setRange(key+0.25,60, Qt::AlignRight);
    ui->customPlot3->replot();

}

void MainWindow::atualizarGraficoTempoDescarga()
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    double value0 = tempoRestante();

    // add data to lines:
    ui->customPlot4->graph(0)->addData(key, value0);

    // set data of dots:
    ui->customPlot4->graph(1)->clearData();
    ui->customPlot4->graph(1)->addData(key, value0);

    ui->customPlot4->graph(0)->rescaleValueAxis();

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot4->xAxis->setRange(key+0.25,60, Qt::AlignRight);
    ui->customPlot4->replot();
}

void MainWindow::on_pushButtonKill_clicked()
{
    int pidInt = ui->lineEditPID->text().toInt();

    if (ehPid(pidInt)){
        string pid = ui->lineEditPID->text().toStdString();
        string aux = "kill -9 ";
        aux = strcat(&aux[0], &pid[0]);
        system(&aux[0]);

        initProcessos();
        int escolhido =  ui->comboBoxTipoProcesso->currentIndex();
        QFile x("processos.json");
        QTextStream xout(&x);
        x.open(QFile::WriteOnly | QFile::Text);
        xout << " ";
        x.flush();
        x.close();
        arquivoMain.open(QFile::WriteOnly | QFile::Text | QIODevice::Append);

        switch (escolhido) {
            case 0:
                filtroCPU(0,0,0);
                break;
            case 1:
                filtroThread(0,0,0);
                break;
            case 2:
                filtroMem(0,0,0);
                break;
            case 3:
                filtroGourmet(0,0,0);
                break;
            default:
                break;
        }

        arquivoMain.flush();
        arquivoMain.close();

        ui->webViewProcessos->reload();
    }
    else
    {
        QMessageBox Msgbox;
        Msgbox.setText("PID INVALIDO!!!");
        Msgbox.exec();
    }
}

void MainWindow::on_pushButtonAtualizar_clicked()
{
    initProcessos();
    int escolhido =  ui->comboBoxTipoProcesso->currentIndex();
    QFile x("processos.json");
    QTextStream xout(&x);
    x.open(QFile::WriteOnly | QFile::Text);
    xout << " ";
    x.flush();
    x.close();
    arquivoMain.open(QFile::WriteOnly | QFile::Text | QIODevice::Append);

    switch (escolhido) {
        case 0:
            filtroCPU(0,0,0);
            break;
        case 1:
            filtroThread(0,0,0);
            break;
        case 2:
            filtroMem(0,0,0);
            break;
        case 3:
            filtroGourmet(0,0,0);
            break;
        default:
            break;
    }

    arquivoMain.flush();
    arquivoMain.close();

    ui->webViewProcessos->reload();
}

