#include "Container.h"

Container::Container()
{
	ObjectName = "Container";

	//Add extra properties here

}
bool Container::OnInit()
{
	// Add initialization code here
	
	return true;
}
bool Container::Close()
{
	// add uninitialization code here
	return true;
}
bool Container::BeginIteration()
{
	// add code to begin iterating here
	
	return true;
}
bool Container::ReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	// add code for reading next record here
	return true;
}

