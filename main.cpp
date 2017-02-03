#include <string>
#include <stdio.h>

#include "model/scheduler.h"
#include "model/node.h"
#include "model/packet.h"


using namespace std;

void test_NAM(){

	NamGenerator::Inst(const_cast<char *>("test.nam"));
        NamGenerator::Inst()->addEvent("V -t * -v 1.0a5 -a 0");
        NamGenerator::Inst()->addEvent("n -t * -a 0 -s 0 -S UP -v circle -c black -i black");
        NamGenerator::Inst()->addEvent("n -t * -a 1 -s 1 -S UP -v box -c tan -i tan");
        NamGenerator::Inst()->addEvent("n -t * -a 2 -s 2 -S UP -v circle -c black -i black");

        NamGenerator::Inst()->addEvent("l -t * -s 0 -d 1 -S UP -r 200000 -D 0.1000 -c black -o 0deg");
        NamGenerator::Inst()->addEvent("l -t * -s 1 -d 2 -S UP -r 200000 -D 0.1000 -c black -o 0deg");

        NamGenerator::Inst()->addEvent("q -t * -s 1 -d 0 -a 0.5");
        NamGenerator::Inst()->addEvent("q -t * -s 0 -d 1 -a 0.5");
        NamGenerator::Inst()->addEvent("q -t * -s 1 -d 2 -a 0.5");
        NamGenerator::Inst()->addEvent("q -t * -s 2 -d 1 -a 0.5");
        NamGenerator::Inst()->addEvent("+ -t 0.000800 -e 1000 -s 0 -d 1 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("- -t 0.000800 -e 1000 -s 0 -d 1 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("h -t 0.000800 -e 1000 -s 0 -d 1 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("r -t 0.140800 -e 1000 -s 0 -d 1 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("+ -t 0.140800 -e 1000 -s 1 -d 2 -c 1 -i 1"); 
        NamGenerator::Inst()->addEvent("- -t 0.140800 -e 1000 -s 1 -d 2 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("h -t 0.140800 -e 1000 -s 1 -d 2 -c 1 -i 1");
        NamGenerator::Inst()->addEvent("r -t 0.280800 -e 1000 -s 1 -d 2 -c 1 -i 1");
}

int main()
{
	//Generar NAM de Prueba
	test_NAM();

	//Planificador
	ListScheduler usim_sched;

	//Nodos
	Node * A = new Node(1, APP, "app layer", 10000);
	Node * S = new Node(1, SIMPLE_SRC, "src iface", 10000);
    Node * D = new Node(2, SIMPLE_END, "dst iface", 10000);
    Node * R = new Node(3, ROUTER, "router", 10000);

	//Enlaces
	Link * A_to_S = new Link(true, 500000, 1, S, R, LINK_NETWORK);
	A->add_interface(A_to_S);
	Link * S_to_R = new Link(true, 500000, 1, S, R, LINK_NETWORK);
	S->add_interface(S_to_R);
	Link * R_to_D = new Link(true, 500000, 1, R, D, LINK_NETWORK);
	R->add_interface(R_to_D);
	
	
	//Nuevo paquete a enviar
	unsigned msg_id = 1;
	Message *msg = new Message(msg_id, PACKETSIZE);

	//Nuevo Evento
	Event * event = new Event(PKT_ARRIVAL_TO_QUEUE,
					static_cast<Pdu *>(msg),
					A,
					S);
	event->set_timestamp(usim_sched.get_curr_time());

	//Insertar evento en planificador
	usim_sched.insert(event);

	//Proximo evento Procesar
	Event * current_event;


        while (usim_sched.get_size() >=1) {
		current_event = usim_sched.dispatch();
		
		if (current_event->get_type()== PKT_ARRIVAL_TO_QUEUE  &&
		    current_event->get_from_node()->get_node_type() == APP)
		{
			current_event->get_to_node()->pdu_arrival(current_event->get_assoc_pdu());

			usim_sched.plan_service(PKT_DEPARTURE_FROM_NODE,
	                                            current_event->get_assoc_pdu(),
	                                            current_event->get_to_node(),
	                                            R);
			cout<<"LLega a S"<<endl;
		}
		

		if (current_event->get_type()== PKT_DEPARTURE_FROM_NODE  &&
		    current_event->get_from_node()->get_node_type() == SIMPLE_SRC)
		{
			Pdu * _pdu = current_event->get_from_node()->pdu_departure();
	                
        	        usim_sched.plan_propagation(_pdu,
							current_event->get_from_node(), 
							current_event->get_to_node());

			cout<<"Sale de S"<<endl;
		}

		if (current_event->get_type()== PKT_ARRIVAL_TO_QUEUE  &&
		    current_event->get_from_node()->get_node_type() == SIMPLE_SRC)
		{
			current_event->get_to_node()->pdu_arrival(current_event->get_assoc_pdu());
	                
        	        usim_sched.plan_service(PKT_DEPARTURE_FROM_ROUTER,
                                                                current_event->get_assoc_pdu(),
                                                                current_event->get_to_node(),
                                                                D);

			cout<<"LLega a R"<<endl;
		}
		if (current_event->get_type()== PKT_DEPARTURE_FROM_ROUTER  &&
		    current_event->get_from_node()->get_node_type() == ROUTER)
		{
			Pdu * _pdu = current_event->get_from_node()->pdu_departure();
	                
        	        usim_sched.plan_propagation(_pdu,
							current_event->get_from_node(), 
							current_event->get_to_node());
			cout<<"Sale de R"<<endl;
		}

		if (current_event->get_type()== PKT_ARRIVAL_TO_QUEUE  &&
		    current_event->get_from_node()->get_node_type() == ROUTER)
		{
			// Consumir el paquete
	                current_event->get_to_node()->increase_received_packets();

	                delete current_event->get_assoc_pdu();
			
			cout<<"LLega a D"<<endl;
		}



	}
}





