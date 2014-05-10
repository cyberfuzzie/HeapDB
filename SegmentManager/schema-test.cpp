#include "schema.pb.h"

#include <string>
#include <fstream>
#include <iostream>

using namespace std;

int main() {

    string name = "Test-Relation";

    schema::Schema s;
    schema::Relation* r = s.add_relations();
    r->set_name(name);
    r->set_segment_id(5);
    r->set_sizeinpages(9);

    fstream output("testrelation", ios::out | ios::binary);
    s.SerializeToOstream(&output);
    output.close();

    fstream input("testrelation", ios::in | ios::binary);
    schema::Schema s2;
    s2.ParseFromIstream(&input);
    input.close();

    assert(s2.relations(0).name() == name);
    cout << "Test ok.";
}
