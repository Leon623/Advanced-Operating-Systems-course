#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <algorithm>


#define CITANJE 0
#define PISANJE 1
#define MAXREAD 20

using namespace std;

bool sortby(const tuple<int, int>& a,
               const tuple<int, int>& b)
{
    if(get<1>(a)==get<1>(b)){

        return (get<0>(a)< get<0>(b));
    }

    else{

        return (get<1>(a) < get<1>(b));

    }

}

vector<int> otvori_cjevovode(int proces, string cjevovodi[]){

    auto pipes = vector<int>();

    for(auto i = 0; i<5; i++){

        string cjevovod = cjevovodi[i];
        if(proces == i){

            auto pfd = open(cjevovod.c_str(),O_RDONLY);
            pipes.emplace_back(pfd);
        }

        else{

            auto pfd = open(cjevovod.c_str(),O_WRONLY);
            pipes.emplace_back(pfd);
        }

    }

    return pipes;
}

void filozof(int i, string cjevovodi[]){
    srand(time(NULL)+i);

    int lokalni_sat = rand()%5;
    typedef vector< tuple<int, int> > tuple_list;
    tuple_list prioritetni_red;
    char message[] = "ZAHTJEV 0,00";


    vector<int> pipes = otvori_cjevovode(i,cjevovodi);
    sleep(1);

    while(1==1){


        tuple<int,int> zahtjev = make_tuple(i,lokalni_sat);
        prioritetni_red.emplace_back(zahtjev);
        sort(prioritetni_red.begin(), prioritetni_red.end(), sortby);
        string zahtjev_string;




        if(lokalni_sat<10){

                zahtjev_string = "ZAHTJEV "+to_string(i)+" "+to_string(lokalni_sat) + "c";

        }

        else{

            zahtjev_string = "ZAHTJEV "+to_string(i)+" "+to_string(lokalni_sat/10) + to_string(lokalni_sat%10);
        }

        cout<<"Filozof "<<i<<" salje ZAHTJEV("<<i<<","<<lokalni_sat<<")"<<endl;

        //lokalni_sat+=1;
        auto desni_filozof = (i+1)%5;
        auto ljevi_filozof = i-1;
        if(ljevi_filozof<0){

            ljevi_filozof = 4;
        }


        write(pipes[desni_filozof],zahtjev_string.c_str(),strlen(message)+1);
        write(pipes[ljevi_filozof],zahtjev_string.c_str(),strlen(message)+1);

        sleep(1);

        int num_of_m = 0;

        while(num_of_m<2 || prioritetni_red.front()!=zahtjev){

            //cout<<"Proces "<<i<<" ima na vrhu reda "<<get<0>(prioritetni_red.front())<<" "<<get<1>(prioritetni_red.front())<<endl;
            char buf[strlen(message)+1] = "";
            read(pipes[i],buf,strlen(message)+1);


            const char usporedba[] = "ca";
            auto timestamp = 1;
            auto proces_id = 1;

            if(buf[strlen(buf)-1]==usporedba[0]){

                timestamp = (int)(buf[strlen(buf)-2])-48;
                proces_id = (int)(buf[strlen(buf)-4])-48;


            }

            else{

                timestamp = ((int)(buf[strlen(buf)-2])-48)*10 + ((int)(buf[strlen(buf)-1]))-48;
                proces_id = (int)(buf[strlen(buf)-4])-48;
            }



            auto oznaka = string (buf);
            oznaka = oznaka.substr(0,7);
            tuple<int,int> poruka(proces_id,timestamp);

            cout<<"Filozof "<<i<<" prima "<<oznaka<<"("<<proces_id<<","<<timestamp<<")"<<endl;

            lokalni_sat = max(lokalni_sat,timestamp)+1;

            if(oznaka=="ZAHTJEV"){


                prioritetni_red.emplace_back(poruka);
                sort(prioritetni_red.begin(), prioritetni_red.end(), sortby);

                string odgovor;

                if(lokalni_sat<10){

                    odgovor = "ODGOVOR "+to_string(i)+" "+to_string(lokalni_sat)+"c";

                }

                else{

                    odgovor = "ODGOVOR "+to_string(i)+" "+to_string(lokalni_sat/10)+to_string(lokalni_sat%10);


                }

                write(pipes[proces_id],odgovor.c_str(),strlen(message)+1);
                cout<<"Filozof "<<i<<" salje ODGOVOR"<<"("<<i<<","<<lokalni_sat<<")"<<endl;


            }

            else if(oznaka=="ODGOVOR"){

                num_of_m++;

//                if((proces_id%5)==i+1){
//
//                    cout<<"FIlozof "<<i<<" uzima desni stapic"<<endl;
//                }
//
//                else{
//
//                    cout<<"Filozof "<<i<<" uzima ljevi stapic"<<endl;
//                }

            }

            else if(oznaka=="IZLAZAK"){


                prioritetni_red.erase(remove(prioritetni_red.begin(),prioritetni_red.end(),poruka), prioritetni_red.end());
            }
//
//            cout<<"Oznaka "<<timestamp<<" proces "<<proces_id<<"zeli "<<oznaka<<endl;
//            lokalni_sat = max(lokalni_sat,timestamp)+1;
//            prioritetni_red.emplace_back(poruka);
//
//            num_of_m++;

        }

        cout<<"Filozof "<<i<<" je usao u kriticni odsjecak!"<<endl;
        sleep(5);
        cout<<"Filozof "<<i<<" je izasao iz kriticnog odsjecka!"<<endl;

        prioritetni_red.erase(remove(prioritetni_red.begin(),prioritetni_red.end(),zahtjev), prioritetni_red.end());

        string izlazak;
        if(get<1>(zahtjev)<10){

            izlazak = "IZLAZAK "+to_string(i)+" "+to_string(get<1>(zahtjev))+ "c";
        }

        else{

           izlazak = "IZLAZAK "+to_string(i)+" "+to_string(get<1>(zahtjev)/10)+ to_string(get<1>(zahtjev)%10);
        }

        cout<<"Filozof "<<i<<" salje IZLAZAK"<<"("<<i<<","<<get<1>(zahtjev)<<")"<<endl;


        write(pipes[desni_filozof],izlazak.c_str(),strlen(message)+1);
        write(pipes[ljevi_filozof],izlazak.c_str(),strlen(message)+1);


//
//        sleep(1);
//        for (int k; k<prioritetni_red.size(); k++){
//
//            cout<<"Ja sam proces "<<i<<" i u prioritetnom redu imam poruku od procesa "<<get<0>(prioritetni_red[k])<<"s vremenskom oznakom "<<get<1>(prioritetni_red[k])<<endl;
//        }

    }



//
//    for(int y = 0; y<5; y++){
//
//    	if(y==i){
//
//            char arr[cjevovodi[y].length() + 1];
//            strcpy(arr, cjevovodi[y].c_str());
//            pfd = open(arr, O_RDONLY);
//            cout<<"Otvorio sam"<<endl;
//            cout<<pfd<<endl;
//
//        }
//
//        else{
//
//            char message[] = "Kroz cijev!";
//            char arr[cjevovodi[y].length() + 1];
//            strcpy(arr, cjevovodi[y].c_str());
//            cout<<"Printam za citanje : "<<arr<<endl;
//            int pmf = open(arr, O_WRONLY);
//            (void) write(pfd, message, strlen(message)+1);
//            cout<<"Procito sam kao"<<endl;
//        }
//
//        (void) read(pfd, buf, MAXREAD);
//        cout<<buf<<endl;
//
//    }

}




//	}

////
////	if(i==1){
////        cout<<"Uso sam u proces 1"<<endl;
////        char message[] = "Kroz cijev!";
////		int pfd = open("./cjev", O_WRONLY);
////        cout<<"Otvorio sam"<<endl;
////        (void) write(pfd, message, strlen(message)+1);
////        cout<<"napiso sam!"<<endl;
////
////	}
//    char message[] = "abssssssssa!\n";
//    char buf[MAXREAD];
//    char penis[40] = "mozaik!";
//	if(i==1){
//
//
//		for(int z = 0; z<4; z++){
//
//
//
//            cout<<"slanje z: "<<slanje[z]<<endl;
//
//            char arr[slanje[z].length() + 1];
//
//            strcpy(arr, slanje[z].c_str());
//            cout<<"Printam za slanje : "<<arr<<endl;
//
//            int pfd = open(arr, O_WRONLY);
//
//            write(pfd, message, strlen(message)+1);
//            cout<<"zavrsio s pisanjem"<<endl;
//            close(pfd);
//
//        }
//
//    }
//
//	if(i==1){
//
//
//        for(int z = 0; z<4; z++){
//
//
//            char arr[primanje[z].length() + 1];
//            strcpy(arr, primanje[z].c_str());
//            cout<<"Printam za citanje : "<<arr<<endl;
//            int pmf = open("./10", O_RDONLY);
//
//            read(pmf, buf, MAXREAD);
//            puts(buf);
//            close(pmf);
//
//        }
//
//        cout<<"zavrsio s citanjem"<<endl;
//
//	}


//}

int main(void){

    int i,pid;
	string cjevovodi[5];
	int k = 0;
	for (int x=0; x<5; x++){

        string cjevovod = "./"+to_string(x);
        cjevovodi[k] = cjevovod;
        k++;

	}
	for(int z = 0; z<5; z++){
		char arr[cjevovodi[z].length() + 1];
        strcpy(arr, cjevovodi[z].c_str());
		cout<<arr<<endl;
		unlink(arr);

        if (mknod(arr, S_IFIFO | 00600, 0)==-1){
            exit(1);
        }


	}


	for (i = 0; i < 5; i++) { //stvaranje procesa pusaca

		switch (pid = fork ()) {

		case -1:
			printf ("R: Ne mogu stvoriti novi proces!\n"); //neuspjesno stvaranje procesa
			exit (1);

		case 0:

			filozof(i,cjevovodi);
			exit (0);

		default:

			break;

		}
	}

	for (int j = 0; j < 5; j++){

        	wait (NULL); //cekaj na procese djece da se nebi stvorili zombiji

	}

	return 0;
}
