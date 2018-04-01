
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

struct RCB;
struct PCB {
	std::string pid;
	int priority;
	std::string state = "Ready";
	PCB * listprev = nullptr;
	PCB * listnext = nullptr;
	PCB * parent = nullptr;
	std::map<RCB*, int> other_resources;
	std::vector<PCB *> children;
};

struct RCB {
	std::string rid;
	int max;
	int available;
	std::map<struct PCB *,int> blocked;
};



struct ProcessList {
	struct PCB* priority0 = new PCB{ "header0" };
	struct PCB* priority1 = new PCB{ "header1" };
	struct PCB* priority2 = new PCB{ "header2" };
};

struct ProcessList* RL = new ProcessList;
struct PCB * currentprocess = nullptr;
RCB* R1 = new RCB{ "R1",1,1 };
RCB* R2 = new RCB{ "R2",2,2 };
RCB* R3 = new RCB{ "R3",3,3 };
RCB* R4 = new RCB{ "R4",4,4 };
std::vector<std::string> all_pids;

struct PCB* insert(struct ProcessList* PL, struct PCB * process) {
	struct PCB* current = nullptr;
	if (process->priority == 0) {
		current = PL->priority0;
		if (current->listnext != nullptr) {
			while (current->listnext != nullptr) {
				current = current->listnext;
			}
		}
		current->listnext = process;
		process->listprev = current;
	}
	else if (process->priority == 1) {
		current = PL->priority1;
		if(current->listnext !=nullptr) {
			while (current->listnext != nullptr) {
				current = current->listnext;
			}
		}
		current->listnext = process;
		process->listprev = current;
	}
	else{
		current = PL->priority2;
		if (current->listnext != nullptr) {
			while (current->listnext != nullptr) {
				current = current->listnext;
			}
		}
		current->listnext = process;
		process->listprev = current;
	}
	current = process;
	return current;
}

int pid_exists(std::string name) {
	for (int i = 0; i < all_pids.size(); ++i) {
		if (name == all_pids[i]) {
			return 1;
		}
	}
	return 0;
}

void remove_pid(std::string name) {
	for (int i = 0; i < all_pids.size(); ++i) {
		if (name == all_pids[i]) {
			all_pids.erase(all_pids.begin() + i);
			break;
		}
	}
}

std::string scheduler() {
	struct PCB* current_process;
	std::string sstream;
	if (RL->priority2->listnext != nullptr) {
		current_process = RL->priority2->listnext;
		sstream = current_process->pid + " ";
		current_process->state = "Running";
		currentprocess = current_process;
		return sstream;
	}
	else if (RL->priority1->listnext != nullptr) {
		current_process = RL->priority1->listnext;
		sstream = current_process->pid + " ";
		current_process->state = "Running";
		currentprocess = current_process;
		return sstream;
	}
	else if(RL->priority0->listnext != nullptr){
		current_process = RL->priority0->listnext;
		sstream = current_process->pid + " ";
		current_process->state = "Running";
		currentprocess = current_process;
		return sstream;
	}
	else {
		return sstream;;
	}
}


struct PCB* Create(std::string name, int priority) {
	struct PCB* result = new struct PCB;
	result->pid = name;
	result->priority = priority;
	if (currentprocess != nullptr) {
		result->parent = currentprocess;
		currentprocess->state = "Ready";
		currentprocess->children.push_back(result);
	}
	all_pids.emplace_back(result->pid);
	result = insert(RL, result);
	return result;
}

int Release(std::string rid, int amount);
void Destroy(std::string name);

//fix destruction for parents children vector
int destroy_tree(PCB * id) {
	if (id == nullptr)
		return 0;
	else {
		while (id->children.size() > 0) {
			Destroy(id->children[0]->pid);
			//id->children.erase(id->children.begin());
		}
		if (id->parent != nullptr) {
			for (int i = 0; i < id->parent->children.size(); i++)
				if (id->parent->children[i] == id) {
					id->parent->children.erase(id->parent->children.begin() + i);
				}
		}
		if (id->listprev != nullptr) {
			id->listprev->listnext = id->listnext;
		}
		if (id->listnext!=nullptr) {
			id->listnext->listprev = id->listprev;
		}
		currentprocess = id;
		for (std::pair<RCB*, int> kv : id->other_resources) {
			Release(kv.first->rid, kv.second);
		}
		remove_pid(id->pid);
		delete id;
		return 0;
	}
}

//Find node by name searching through lists, then call destroy tree
void Destroy(std::string name) {
	PCB* current = nullptr;
	int counter = 0;
	int found = 0;
	PCB* target = RL->priority0->listnext;
	while (counter <= 2) {
		if (counter == 1)
			target = RL->priority1->listnext;
		else if (counter == 2)
			target = RL->priority2->listnext;
		while (target != nullptr) {
			current = target;
			if (current->pid == name) {
				destroy_tree(current);
				found = 1;
				return;
			}
			target = target ->listnext;
		}
		counter++;
	}
	if (found == 0) {
		counter = 1;
		RCB* resource = nullptr;
		while (counter <= 4) {
			if (counter == 1)
				resource = R1;
			else if (counter == 2)
				resource = R2;
			else if (counter == 3)
				resource = R3;
			else if (counter == 4)
				resource = R4;
			for (std::pair<PCB*, int> kv : resource->blocked) {
				if (kv.first->pid == name) {
					PCB* temp = kv.first;
					resource->blocked.erase(kv.first);
					destroy_tree(kv.first);
					found = 1;
					return;
				}
			}
			counter++;
		}

	}
}


RCB* get_rcb(std::string rid) {
	if (rid == "R1")
		return R1;
	else if (rid == "R2") {
		return R2;
	}
	else if (rid == "R3") {
		return R3;
	}
	else if (rid == "R4") {
		return R4;
	}
	return nullptr;
}

void remove_from_list(ProcessList * PL, PCB* process) {
	int p = process->priority;
	PCB * temp = nullptr;
	switch (p) {
	case 2 :
		temp = PL->priority2;
		break;
	case 1:
		temp = PL->priority1;
		break;
	case 0:
		temp = PL->priority0;
		break;
	}
	while (temp != nullptr) {
		if (temp->pid == process->pid) {
			temp->listprev->listnext = temp->listnext;
			if (process->listnext != nullptr)
				temp->listnext->listprev = temp->listprev;
			temp->listprev = nullptr;
			temp->listnext = nullptr;
		}
		temp = temp->listnext;
	}
}

int Request(std::string rid, int amount) {
	RCB* r = get_rcb(rid);
	if (r == nullptr || amount > r->max ||amount + currentprocess->other_resources[r] > r->max) {
		//std::cout << "Request is invalid" << std::endl;
		return 0;
	}
	if (r->available >= amount) {
		r->available = r->available - amount;
		currentprocess->other_resources[r] += amount;
		return 1;
	}
	else {
		currentprocess->state = "Blocked";
		r->blocked[currentprocess] =amount;
		//std::cout << "*Process " << currentprocess->pid << " is blocked; ";
		remove_from_list(RL, currentprocess);
		return 1;
	}
}

int Release(std::string rid, int amount) {
	RCB* r = get_rcb(rid);
	if (r == nullptr) {
		//std::cout << "resource " << rid << "does not exist"<<std::endl;
		return 0;
	}

	if (currentprocess->other_resources[r] < amount) {
		//std::cout << "current process " << currentprocess->pid << " does not hold " << amount
			//<< " of resource " << r->rid << std::endl;
		return 0;
	}
	r->available += amount;
	currentprocess->other_resources[r] = currentprocess->other_resources[r] - amount;
	int req = 0;
	while (r->available >= req) {
		if (r->blocked.size() == 0)
			break;
		if (r->blocked.begin()->first == nullptr)
		{
			r->blocked.erase(r->blocked.begin());
			continue;
		}
		PCB* current = r->blocked.begin()->first;
		req = r->blocked.begin()->second;
		if (r->available >= req) {
			r->available = r->available - req;
			r->blocked.begin()->first->other_resources[r] = req;
			r->blocked.erase(r->blocked.begin());
			current->state = "Ready";
			currentprocess->state = "Ready";
			insert(RL, current);
		}
	}
	return 1;
}

void Time_out() {
	remove_from_list(RL, currentprocess);
	currentprocess->state = "Ready";
	insert(RL, currentprocess);
}

int is_descendent(PCB* process, std::string name) {
	if (name == process->pid)
		return 1;
	for (int i = 0; i < process->children.size();i++) {
		if (is_descendent(process->children[i], name) == 1)
			return 1;
	}
	return 0;
}

void shell() {
	std::string fname;
	std::string fname2";
	std::cout << "path to input file?: ";
	std::cin >> fname;
	std::cout << "path to output file?: ";
	std::cin >> fname2;
	std::ifstream ourinput(fname);
	std::ofstream ouroutput(fname2);
	std::string buf;
	ouroutput << scheduler();
	while (std::getline(ourinput, buf)) {
		std::string a = "";
		std::string b = "";
		std::string c = "";

		int counter = 0;
		for (int i = 0; i < buf.length();i++) {
			if (buf[i] == ' ')
				counter++;
			else if (counter == 0)
				a.push_back(buf[i]);
			else if (counter == 1)
				b.push_back(buf[i]);
			else
				c.push_back(buf[i]);
		}

		if (buf == "init") {
			Destroy("init");
			currentprocess = nullptr;
			Create("init", 0);
			ouroutput << std::endl;
			ouroutput << scheduler();
		}
		else if (a == "cr") {
			if (b == "" || pid_exists(b)||b.length()>1)
				ouroutput << "error ";
			else if (c == "0" || c == "1" || c == "2") {
				Create(b, std::stoi(c));
				ouroutput << scheduler();
			}
			else
				ouroutput << "error ";
		}
		else if (a == "de") {
			if (c != "" || b == "")
				ouroutput << "error ";
			else if (pid_exists(b) == 0 || b == "init"|| is_descendent(currentprocess,b)==0)
				ouroutput << "error ";
			else {
				Destroy(b);
				ouroutput << scheduler();
			}
		}
		else if (a == "req") {
			if (c == "" || b == "")
				ouroutput << "error ";
			else if (c == "1" || c == "2" || c == "3" || c == "4") {
				if (Request(b, std::stoi(c)) == 0) {
					ouroutput << "error ";
				}
				else {
					ouroutput << scheduler();
				}
			}
			else
				ouroutput << "error ";
		}
		else if (a == "rel") {
			if (c == "" || b == "")
				ouroutput << "error ";
			else if (c == "1" || c == "2" || c == "3" || c == "4") {
				if (Release(b, std::stoi(c)) == 0) {
					ouroutput << "error ";
				}
				else {
					ouroutput << scheduler();
				}
			}
			else
				ouroutput << "error ";
		}
		else if (buf == "to") {
			Time_out();
			ouroutput << scheduler();
		}
		else if (buf == "")
			continue;
		else {
			ouroutput << "error ";
		}
		std::string buf = "";
	}
}

int main() {

	PCB* init = Create("init", 0);
	shell();
	return 0;
}
