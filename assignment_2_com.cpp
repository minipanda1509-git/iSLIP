//Header file Declaration
#include<bits/stdc++.h>

using namespace std;

//Global variable Declaration as default values
int switch_port_count = 8; //number of switch port count 
int buffer_size = 4; //buffer size 
float packet_gen_prob = 0.5; //packet generation probabiolity
char queue_type[20] = "INQ"; //queue type
int max_time_slots = 10000; //number of time slots for simulation
float k_knockout = 0.6; //value of K(defalut is 0.6)
string out_file = "out.txt"; //default output file

//structure to store information of packet
typedef struct packet {
    bool is_generated; //if packet is generated or not
    int ip; //input port
    int op; //output port
    double gen_time; //geneartion tym of packet
    double comp_time; //completion time of transmission
    //We will calculate delay by differenece of above two
}
packet;

//Function to Generate a Packet
// input params - ip: input port , t: timeslot in which func is called , p : prob of gen of packet
//return op : packet 
//
packet generate_packet(int ip, int t, float p) {
    packet pkt;
    double r = ((double) rand() / (RAND_MAX)); //genearte a random no. between 0 and 1
    int op = rand() % switch_port_count; //selecting a random output port 
    double time = t + (double)((rand() % 10 + 1) / 1000.0); //generation time selected randomly between t+0.001 to t+0.01
    if (r >= p)
        pkt.is_generated = false; //if random no. is greater than p so packet will not be generated
    else {
        pkt.is_generated = true; //else packet will be generated
    }
    //setting claculated params to packet parameteers
    pkt.ip = ip;
    pkt.gen_time = time;
    pkt.op = op;
    //returning packet
    return pkt;
}

//INQ function
void INQ() {

    int delay = 0; //delay variable that stores delay sum over all simulation time
    int no_transmitted_packet = 0; //to store number of transmitted packet overall
    int no_gen_pack = 0; //to store number of genearetd packet
    vector < int > delay_arr; //delay vector to claculate std. deviation
    int util[switch_port_count] = {
        0
    }; //array to calculate link utilization

    queue < packet > ip_q[switch_port_count]; //ip port queues(with size buffer_size)
    queue < packet > op_q[switch_port_count]; //op port queues(with size 1 so basically there is no op port queue)
    srand(time(0));
    for (int t = 1; t < max_time_slots; t++) { //loop is running for overall simulation time

        bool can_transmit[switch_port_count] = {
            0
        }; //array to find whether we can transmit at that op port or not

        for (int i = 0; i < switch_port_count; i++) {
            //for each input port we are generating a packet 
            packet p;
            p = generate_packet(i, t, packet_gen_prob);
            if (p.is_generated) {
                no_gen_pack++; //if packet is genearted then we r putting in input queue
                if (ip_q[i].size() < buffer_size)
                    ip_q[i].push(p);

            }
            if (ip_q[i].size() > 0) //checking whether queue is not empty
                if (can_transmit[ip_q[i].front().op] == false) { //checking for packet contention is it is not colliding then directly transmitting
                    if (op_q[ip_q[i].front().op].size() < 1) {
                        no_transmitted_packet++;
                        op_q[ip_q[i].front().op].push(ip_q[i].front());
                        util[ip_q[i].front().op]++;
                        can_transmit[ip_q[i].front().op] = true; //marking that we cant transmit at corresponding op port in this time slot
                        ip_q[i].pop();
                    }
                }
        }
        //processsing the packet
        for (int i = 0; i < switch_port_count; i++) {
            if (op_q[i].size() > 0) {
                packet p = op_q[i].front();
                p.comp_time = t + 1; //setting comp time of packet
                op_q[i].pop(); //popping out from op queue since it is processed

                delay_arr.push_back((int) p.comp_time - (int) p.gen_time); //pushing in the delay array

            }
        }
    }
    long long sum = 0;
    for (int i = 0; i < switch_port_count; i++) {
        sum += util[i]; //summing to get utizilation
    }

    ofstream fout;
    fout.open(out_file, ios::app); //opening the file in append mode

    for (int i = 0; i < delay_arr.size(); i++) {

        delay += delay_arr[i]; //calculating overall delay
    }

    double Average_delay = (delay * 1.0) / delay_arr.size(); //calculating average dealy for overall simulation

    double stand_dev = 0; //variable to store standard deviation

    for (int i = 0; i < delay_arr.size(); i++) {

        stand_dev += pow(((double) delay_arr[i] - Average_delay), 2);
    }

    stand_dev = sqrt(stand_dev / delay_arr.size()); //standard deviation calculated

    float link_utiliization = sum / (switch_port_count * max_time_slots * 1.0); //calculaitng link utilization
    //inserting the entries in the output file
    fout << "\n" << switch_port_count << "\t" << packet_gen_prob << "\t" << queue_type << "\t\t\t" << Average_delay << "\t\t" << stand_dev << "\t\t" << link_utiliization;

    fout.close(); //closing the file
}

//comaprator function to sort packet on basis of generation time
bool cmp(packet p1, packet p2) {
    return (p1.gen_time < p2.gen_time);
}

//function of KOUQ scheduling
void KOUQ() {
    double drop_prob = 0; //variable to store drop prob
    int delay = 0; //delay variable that stores delay sum over all simulation time
    int no_transmitted_packet = 0; //to store number of transmitted packet overall
    int no_gen_pack = 0; //to store number of genearetd packet
    vector < int > delay_arr; //delay vector to claculate std. deviation
    int util[switch_port_count] = {
        0
    }; //array to calculate link utilization

    queue < packet > ip_q[switch_port_count]; //ip port queues(with size 1)
    queue < packet > op_q[switch_port_count]; //op port queues(with size K)
    srand(time(0) + 79);
    for (int t = 1; t < max_time_slots; t++) { //loop is running for overall simulation time

        int can_transmit[switch_port_count] = {
            0
        }; //array to find whether we can transmit at that op port or not
        vector < packet > curr_pack; //vector to store  gen packet in  current time slot
        for (int i = 0; i < switch_port_count; i++) {
            //for each input port we are generating a packet 
            packet p;
            p = generate_packet(i, t, packet_gen_prob);
            if (p.is_generated) {
                no_gen_pack++; //if packet is genearted then we r putting in input queue
                if (ip_q[i].size() < 1)
                    ip_q[i].push(p);
            }

            if (ip_q[i].size() > 0)

            {
                curr_pack.push_back(ip_q[i].front()); //pushing the current packets in cuur_pack vector

            }
        }
        sort(curr_pack.begin(), curr_pack.end(), cmp); //sorting the entries in increasing order of gen time

        for (int i = 0; i < curr_pack.size(); i++) {
            if (can_transmit[curr_pack[i].op] < (int)(k_knockout * switch_port_count)) { // if we transmit the packet(count<k*N)
                if (op_q[curr_pack[i].op].size() < buffer_size) {
                    no_transmitted_packet++; //increasing transmitted packet by 1
                    op_q[curr_pack[i].op].push(curr_pack[i]);
                    util[curr_pack[i].op]++;
                    can_transmit[curr_pack[i].op]++;
                    ip_q[curr_pack[i].ip].pop();
                }
            } else {
                drop_prob++; //otherwise packet is grater than K so it contributes to droppping prob
            }

        }
        //PaCket Processing
        for (int i = 0; i < switch_port_count; i++) {
            if (op_q[i].size() > 0) {
                packet p = op_q[i].front();
                p.comp_time = t + 1; //setting completion time of packet
                op_q[i].pop(); //popping that packet from op queue
                delay += (int) p.comp_time - (int) p.gen_time; //calculating delay
                delay_arr.push_back((int) p.comp_time - (int) p.gen_time); //pushing delay into delay array

            }
        }
    }
    long long sum = 0;
    for (int i = 0; i < switch_port_count; i++) {
        sum += util[i]; //calculating link utilization
    }
    float Average_delay = delay / (no_transmitted_packet * 1.0); //calculating Overall delay

    float stand_dev = 0; //to store std deviation of delay
    for (int i = 0; i < delay_arr.size(); i++) {
        stand_dev += (delay_arr[i] - Average_delay) * (delay_arr[i] - Average_delay);
    }

    ofstream fout;
    fout.open(out_file, ios::app); //opening output file in append mode

    stand_dev = sqrt(stand_dev / delay_arr.size()); //calculating standard deviation

    float link_utiliization = sum / (switch_port_count * max_time_slots * 1.0); //calculating link utilizatiomn
    //inserting entries into output file
    fout << "\n" << switch_port_count << "\t" << packet_gen_prob << "\t" << queue_type << "\t\t" << Average_delay << "\t\t" << stand_dev << "\t\t" << link_utiliization;

    fout.close();
    drop_prob = drop_prob / max_time_slots; //calculting drop probabaility
    cout << "\n Drop Probability::" << drop_prob / switch_port_count;
}

void Islip() {
    //declaration
    double drop_prob = 0; //variable to store drop prob
    int delay = 0; //delay variable that stores delay sum over all simulation time
    int no_transmitted_packet = 0; //to store number of transmitted packet overall
    int no_gen_pack = 0; //to store number of genearetd packet
    vector < int > delay_arr; //delay vector to claculate std. deviation
    queue < packet > ip_q[switch_port_count][switch_port_count];
    int a_p[switch_port_count] = { //Accept pointer array
        0
    };
    int g_p[switch_port_count] = { //Grant Pointer array
        0
    };
    srand(time(0));
    for (int t = 0; t < max_time_slots; t++) {
        cout<<"Round : "<<t<<endl;
        int R[switch_port_count][switch_port_count]; //Array to maintain requests from input-port
        int G[switch_port_count] = { //Array to maintain Grant-requests
            -1
        };
        int A[switch_port_count] = { //Array to Accept Requests
            -1
        };
        bool op_flag[switch_port_count] = { //Array to check whether the give output-port connected or not
            false
        };
        bool ip_flag[switch_port_count] = { //Array to check whether given input port has established connection
            false
        };
        //generate packets for each input-port in each time-slot
        for (int ip = 0; ip < switch_port_count; ip++) {
            packet p = generate_packet(ip, t, packet_gen_prob); //generate packet
            if (p.is_generated) { //check if packet is generated to insert packet in queue
                no_gen_pack++; //Maintain count of generated packets
                ip_q[ip][p.op].push(p); //Push the packet in specified queue
            }
        }
        int conn = 0, iter = 0; 
        do {
            conn = 0;
            //Build Requests
            cout<<"Iter : "<<iter<<endl;
            cout<<"Requested Connections : "<<endl;
            for (int ip = 0; ip < switch_port_count; ip++) {
                for (int op = 0; op < switch_port_count; op++) {
                    if (ip_flag[ip] || op_flag[op]) { // Check if any of ports already established connection
                        R[ip][op] = -1;
                        continue;
                    }
                    if (!ip_q[ip][op].empty()) { // Check whether queue has packet
                        R[ip][op] = 1; //Make request
                        cout<<ip<<" : "<<op<<endl;
                    } else R[ip][op] = -1; 
                }
            }

            //Grant Requests
            cout<<"Granted connections : "<<endl;
            for (int op = 0; op < switch_port_count; op++) {
                if (op_flag[op]) continue; //Check if output-port established connection
                int i = 0, ip = g_p[op];
                for (; i < switch_port_count; i++) { //Check for next avilable request
                    if (R[ip][op] == 1) break; // Break if request found
                    ip = (ip + 1) % switch_port_count;
                }
                if (i < switch_port_count) G[op] = ip,cout<<op<<" : "<<ip<<endl; //Grant to first avilable request
                else G[op] = -1;
            }

            //Accept-phase
            cout<<"Accepted connections : "<<endl;
            for (int ip = 0; ip < switch_port_count; ip++) {
                if (ip_flag[ip]) continue; //Check if input-port is busy
                int i = 0, op = a_p[ip];
                for (; i < switch_port_count; i++) { //Check for first port that granted request
                    if (G[op] == ip) break; //Break if Grant found
                    op = (op + 1) % switch_port_count;
                }
                if (i < switch_port_count) {
                    conn++; //maintain no.of connections established in each iteration
                    no_transmitted_packet++; //Count no.of packets transmitted
                    cout<<ip<<" : "<<op<<endl;
                    op_flag[op] = true; // Mark outport as established connection
                    ip_flag[ip] = true; // Mark inputport as establishe connection
                    packet p = ip_q[ip][op].front(); //store the front packet in the queue
                    p.comp_time = t + 1; //store completion time
                    delay += (int) p.comp_time - (int) p.gen_time; //calculate total delay till now
                    delay_arr.push_back((int) p.comp_time - (int) p.gen_time); //maintain delay of each packet
                    ip_q[ip][op].pop(); //remove packet from queue
                    if( iter == 0 ){
                        a_p[ip] = (op+1)%switch_port_count;
                        g_p[op] = (ip+1)%switch_port_count;
                    }
                }
            }
            iter++;
        } while (conn > 0);
    }
    float Average_delay = delay / (no_transmitted_packet * 1.0); //Calculate average delay
    // Calculate Standard-Deviation
    float stand_dev = 0;
    for (int i = 0; i < delay_arr.size(); i++) {
        stand_dev += (delay_arr[i] - Average_delay) * (delay_arr[i] - Average_delay); 
    }
    stand_dev = sqrt(stand_dev / delay_arr.size());
    // calculate link utilisation
    float link_utiliization = no_transmitted_packet / (switch_port_count * max_time_slots * 1.0);
    ofstream fout;
    fout.open(out_file, ios::app);
    //float link_utiliization = sum / (switch_port_count * max_time_slots * 1.0);
    //cout << "N\tp\tQueueType\tavgPD\t\tstd_dev_pd\tavg_link_util\n";
    cout<<no_gen_pack<<"\t"<<no_transmitted_packet<<endl;
    fout << "\n" << switch_port_count << "\t" << packet_gen_prob << "\t" << queue_type << "\t\t" << Average_delay << "\t\t" << stand_dev << "\t\t" << link_utiliization;
    cout << "\n" << switch_port_count << "\t" << packet_gen_prob << "\t" << queue_type << "\t\t" << Average_delay << "\t\t" << stand_dev << "\t\t" << link_utiliization;
    fout.close();
}

int main(int argc, char ** argv) {
    //  cout<<"IN MAIN";
    // ./routing −N switchportcount −B buffersize −p packetgenprob −queue INQ −K knockout −out outputfile −T maxtimeslots 

    // parsing input 
    if (argc <= 15) {
        for (int i = 1; i < argc; i += 2) {
            if (strcmp(argv[i], "-N") == 0) {
                switch_port_count = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-B") == 0) {
                buffer_size = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "-p") == 0) {
                packet_gen_prob = stof(argv[i + 1]);
            } else if (strcmp(argv[i], "-queue") == 0) {
                int j = 0;
                do {
                    queue_type[j] = (argv[i + 1])[j], j++;
                }
                while ((argv[i + 1])[j - 1] != '\0');
            } else if (strcmp(argv[i], "-k") == 0) {
                k_knockout = stof(argv[i + 1]);
            } else if (strcmp(argv[i], "-out") == 0) {
                out_file = argv[i + 1];
            } else if (strcmp(argv[i], "-T") == 0) {
                max_time_slots = stoi(argv[i + 1]);
            } else {
                cout << "Wrong Input Type" << endl;
                return 0;
            }

        }

        if (strcmp("INQ", queue_type) == 0) {
            INQ();
            cout << "\n Program run succesfully...\n check " << out_file << " for result...\n";
        } else if (strcmp("KOUQ", queue_type) == 0) {
            KOUQ();
            cout << "\n Program run succesfully...\n check " << out_file << " for result...\n";
        } else if (strcmp("ISLIP", queue_type) == 0) {
            Islip();
            cout << "\n Program run succesfully...\n check " << out_file << " for result...\n";
        } else {
            cout << "Queue can be only INQ / KOUQ / ISLIP";
            cout << "\nProgram is terminating";
            return 0;
        }

    } else {
        cout << "::Please provide proper Arguments::" << endl;
        cout << "output must be of form:- \n./routing N switchportcount B buffersize p packetgenprob queue INQ K knockout out outputfile T maxtimeslots ";
        cout << "\nProgram is Terminating...";

    }

    return 0;
}
