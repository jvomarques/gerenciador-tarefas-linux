#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <bits/stdc++.h>
#include <dirent.h>

#include <QLineEdit>
#include <QTextStream>
#include <QIODevice>
#include <QCoreApplication>
#include <qcoreapplication.h>
#include <QtCore>
#include <QFile>
#include <qiodevice.h>
#include <QFileInfo>
#include <QFileInfoList>
#include <qfileinfo.h>
#include <QString>
#include <qiodevice.h>
#include <QDebug>
#include <QMessageBox>
#include <qmessagebox.h>

using namespace std;

string IntToString(int x){
    return static_cast<ostringstream*>( &(ostringstream() << x) )->str();
}

class Proc{
public:
    QString nome;
    int pid, ppid;
    float cpu, mem, thread, gourmet;

    vector< Proc > filhos;

    void setPid(int p){
        this->pid = p;

        if (p >= 0){
            this->ppid = getPai(p);
            this->nome = getNome(p);
            this->cpu = sizeCPU(p);
            this->mem = sizeMem(p);
            this->thread = sizeThread(p);
            this->gourmet = sizeGourmet(p);
        }
    }
    int getPai(int n)
    {
        int aux2;

        QString a;
        QStringList b;
        QString pid = QString::number(n);
        QFile file("/proc/" + pid + "/stat");
        if(file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            do
            {
                QTextStream stream_in(&file);
                a = stream_in.readLine().simplified();
                b = a.split(' ', QString::SkipEmptyParts);
                if((int)b.size() != 0)
                {
                    QString resp = b.at(3);
                    aux2 = resp.toInt();
                }
            }while(!a.isNull());
            file.close();
        }
        return aux2;
    }

    QString getNome(int n)
    {
        QString pid = QString::number(n);

        string aux2 = "";

        QString a;

        QStringList b;

        QFile file("/proc/" + pid + "/stat");

        if(file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            do
            {
                QTextStream stream_in(&file);

                a = stream_in.readLine().simplified();

                b = a.split(' ', QString::SkipEmptyParts);
                if((int)b.size() != 0)
                {
                    QString resp = b.at(1);
                    string resp1 = resp.toStdString();
                    for(int i=0; i<(int)resp1.size()-1; i++)
                        resp1[i] = resp1[i+1];

                    for(int i=0; i<(int)resp1.size()-2; i++)
                        aux2 += resp1[i];
                }
            }while(!a.isNull());

            file.close();
        }
        if(aux2 == "")
            aux2 = "processos";

        return QString::fromUtf8(aux2.c_str());
    }

    double sizeCPU(int k)
    {
        int time_total1=0;
        QString line;
        QStringList vline;
        QFile in("/proc/stat");
        in.open(QIODevice::ReadOnly | QIODevice::Text);
        if(in.exists()){
            QTextStream stream_in(&in);
            line = stream_in.readLine().simplified();
            vline = line.split(' ', QString::SkipEmptyParts);
            if((int)vline.size() != 0){
                for(int i=1; i<11; i++){
                    time_total1 += vline.at(i).toInt();
                }
            }
        }
        in.close();

        int stime1=0;
        QString s = QString::number(k);
        QFile inz("/proc/"+s+"/stat");
        inz.open(QIODevice::ReadOnly | QIODevice::Text);
        if(inz.exists()){
            QTextStream stream_in(&inz);
            line = stream_in.readLine().simplified();
            vline = line.split(' ', QString::SkipEmptyParts);
            if((int)vline.size() != 0){
                stime1 += vline.at(14).toInt();
            }
        }
        inz.close();
        if(time_total1 == 0) return 1.0;
        return (100.0*stime1)/time_total1;
    }

    int sizeMem(int n){

        int aux2;

        QString resp;
        QString a;
        QStringList b;

        QString pid = QString::number(n);

        QFile file("/proc/" + pid + "/stat");
        if(file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            do
            {
                QTextStream stream_in(&file);
                a = stream_in.readLine().simplified();
                b = a.split(' ', QString::SkipEmptyParts);
                if((int)b.size() != 0)
                        resp = b.at(22);
            }while(!a.isNull());

            file.close();
        }

        aux2 = resp.toInt();
        return aux2;
    }

    int sizeThread(int n){

        int aux2=0;

        QString a;
        QStringList b;
        QString pid = QString::number(n);

        QFile file("/proc/" + pid + "/stat");

        if(file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            do
            {
                QTextStream stream_in(&file);
                a = stream_in.readLine().simplified();
                b = a.split(' ', QString::SkipEmptyParts);
                if((int)b.size() != 0)
                {
                    QString resp = b.at(19);
                    aux2 = resp.toInt();
                }
            }while(!a.isNull());

            file.close();
        }
        return aux2;
    }

    int sizeGourmet(int n){
        return sizeCPU(n)+sizeThread(n)+sizeMem(n)/3;
    }
};
