#ifndef _SEG_GIF_H
#define _SEG_GIF_H

#include "_seg_GIF.h"

#include <cassert>

// ************ XML FUNCTIONALITY
char * xmlReadFile(string filename)
{
    ifstream myXMLfile(filename.c_str());
    stringstream buffer;
    buffer << myXMLfile.rdbuf();
    string tmpstring=buffer.str();

    char * tmpcharptr=new char[tmpstring.size()+1];
    tmpcharptr[tmpstring.size()]='\0';
    memcpy(tmpcharptr,tmpstring.c_str(),tmpstring.size());
    return tmpcharptr;
}

xml_document<> * xmlReadFromCharPtr(char * file_text)
{

    xml_document<> * doc=new xml_document<>;
    doc->parse<0>(file_text);
    return doc;
}

rapidxml::xml_attribute<> * xmlAddAtributeToItem(rapidxml::xml_document<> * baseDocument, rapidxml::xml_node<> * node, std::string nameString, std::string valueString=""){
  char * valueChar= new char[valueString.length()];
  strcpy(valueChar,valueString.c_str());
  char * nameChar= new char[nameString.length()];
  strcpy(nameChar,nameString.c_str());
  rapidxml::xml_attribute<> *newAtribute = baseDocument->allocate_attribute(nameChar, valueChar);
  node->append_attribute(newAtribute);
  return newAtribute;
}

rapidxml::xml_node<> * xmlAddNodeToItem(rapidxml::xml_document<> * baseDocument, rapidxml::xml_node<> * node, std::string nameString, std::string valueString=""){
  char * valueChar= new char[valueString.length()];
  strcpy(valueChar,valueString.c_str());
  char * nameChar= new char[nameString.length()];
  strcpy(nameChar,nameString.c_str());
  rapidxml::xml_node<> * newNode = baseDocument->allocate_node(rapidxml::node_element, nameChar, valueChar);
  node->append_node(newNode);
  return newNode;
}

std::string xmlGetTextFromXmlNode(rapidxml::xml_node<> * current_node,
                               const char * fieldName,
                               bool * exists)
{
    string curval="";
    if(current_node->first_node(fieldName)!=NULL)
    {
        string curval=current_node->first_node(fieldName)->value();
        if(curval.size()>0)
        {
            exists[0]=true;
            return curval;
        }
        else
        {
            exists[0]=true;
            return curval;
        }
    }
    else
    {
        exists[0]=false;
        return curval;
    }
}

std::string xmlGetNameFromItemWithNumber(rapidxml::xml_node<> * current_node,
                               int input_number,
                               bool * exists)
{
  for (rapidxml::xml_node<> *inner_node=current_node->first_node(); inner_node; inner_node=inner_node->next_sibling())
  {
    if(strcmp(inner_node->name(),"item")==0)
    {
      bool inner_exists=0;
      int inner_number=atoi(xmlGetTextFromXmlNode(inner_node,"number",&inner_exists).c_str());
      if(inner_number==input_number){
        exists[0]=true;
        return xmlGetTextFromXmlNode(inner_node,"name",&inner_exists);
      }
    }
  }
  return std::string("");
}

std::string xmlGetTextFromXmlAttribute(rapidxml::xml_node<> * current_node,
                               const char * fieldName,
                               bool * exists)
{
    std::string curval="";
    if(current_node->first_attribute(fieldName)!=NULL)
    {
        std::string curval=current_node->first_attribute(fieldName)->value();
        if(curval.size()>0)
        {
            exists[0]=true;
            return curval;
        }
        else
        {
            exists[0]=true;
            return curval;
        }
    }
    else
    {
        exists[0]=false;
        return curval;
    }
}

// ************ Smartpointer Class
template <class T>
smartPointer<T>::smartPointer() : pData(0), reference(0)
{
    reference = new RefCounter();
    reference->AddRef();
}

template <class T>
smartPointer<T>::smartPointer(T *pValue) : pData(pValue), reference(0)
{
    reference = new RefCounter();
    reference->AddRef();
}

template <class T>
smartPointer<T>::smartPointer(const smartPointer<T> &sp) : pData(sp.pData), reference(sp.reference)
{
    reference->AddRef();
}

template <class T>
T *smartPointer<T>::cPTR()
{
    return pData;
}

template <class T>
smartPointer<T>::~smartPointer()
{
    if(reference->Release() == 0)
    {
        delete pData;
        delete reference;
    }
}

template <class T>
T &smartPointer<T>::operator*()
{
    return *pData;
}

template <class T>
T *smartPointer<T>::operator->()
{
    return pData;
}

template <class T>
void smartPointer<T>::printCounter()
{
    cout<<this->reference->getCount()<<endl;
}

template <class T>
smartPointer<T> &smartPointer<T>::operator =(const smartPointer<T> &sp)
{
    if (this != &sp)
    {
        if(reference->Release() == 0)
        {
            delete pData;
            delete reference;
        }
        pData = sp.pData;
        reference = sp.reference;
        reference->AddRef();
    }
    return *this;
}


// ************ DATASET Structure
niftiObject::niftiObject()
{
    this->file=NULL;
}

//Takes ownership of the data
niftiObject::niftiObject(nifti_image * nifti_img)
{
    if(nifti_img!=NULL && nifti_img->fname!=NULL)
    {
        this->filename=nifti_img->fname;
    }
    else
    {
        this->filename="";
    }
    if(nifti_img!=NULL)
    {
        this->file=nifti_img;
        this->file->scl_inter=0;
        this->file->scl_slope=1;
        if(this->file->data!=NULL)
        {
            if(this->file->datatype!=NIFTI_TYPE_FLOAT32)
                seg_changeDatatype<float>(this->file);
            nifti_update_dims_from_array(this->file);
        }
    }
    else
    {
        this->file=NULL;
    }
}

niftiObject::~niftiObject()
{
    if(this->file!=NULL) nifti_image_free(this->file);
}


// The dataset object owns the data. Do not delete/free the ptr.
nifti_image * niftiObject::getPtr()
{
    if(this!=NULL)
    {
        if(this->file!=NULL){
            nifti_update_dims_from_array(this->file);
            this->file->scl_inter=0;
            this->file->scl_slope=1;
        }
        return this->file;
    }
    else
    {
        return NULL;
    }
}

string niftiObject::getFilename()
{
    return this->filename;
}
void niftiObject::setFilename(string _filename)
{
    this->filename=_filename;
    nifti_set_filenames(this->getPtr(),_filename.c_str(),0,0);
}

// Dynamic loading image

niftiDynamicObject::niftiDynamicObject()
{
    this->filename="0";
    this->file=NULL;
}

//Takes ownership of the data if own is 1, otherwise, makes copy
niftiDynamicObject::niftiDynamicObject(nifti_image * nifti_img, bool _own)
{
    if(nifti_img!=NULL && nifti_img->fname!=NULL)
    {
        this->filename=nifti_img->fname;
    }
    else
    {
        this->filename="";
    }
    if(nifti_img!=NULL)
    {
        if(_own)
        {
            this->file=nifti_img;
            if(this->file->data!=NULL)
            {
                if(this->file->datatype!=NIFTI_TYPE_FLOAT32)
                    seg_changeDatatype<float>(this->file);
                nifti_update_dims_from_array(this->file);
            }
        }
        else
        {
            this->file=nifti_copy_nim_info(nifti_img);
            nifti_update_dims_from_array(this->file);
            this->file->datatype = NIFTI_TYPE_FLOAT32;
            this->file->nbyper = sizeof(float);
            this->file->data=(void *)malloc(this->file->nvox * this->file->nbyper);
            memcpy(this->file->data, nifti_img->data, this->file->nvox * this->file->nbyper);
            nifti_update_dims_from_array(this->file);

        }
    }
    else
    {
        this->file=NULL;
    }
}

niftiDynamicObject::niftiDynamicObject(string _filename,
                                       bool read_data)
{
    this->filename=_filename.c_str();
#ifndef NDEBUG
    cout << "[NiftySeg DEBUG] Reading image "<< this->filename<<endl;
#endif
    this->file=nifti_image_read(_filename.c_str(),read_data);
    nifti_free_extensions(this->file);
    if(this->file==NULL)
    {
        cerr << "ERROR:Could not read or allocate image "<< this->filename.c_str()<< endl;
        exit(1);
    }
    this->file->scl_inter=0;
    this->file->scl_slope=1;

    if(this->file->data!=NULL)
    {
        if(this->file->datatype!=NIFTI_TYPE_FLOAT32)
            seg_changeDatatype<float>(this->file);
        nifti_update_dims_from_array(this->file);
    }
}

nifti_image * niftiDynamicObject::getPtr()
{
    if(this!=NULL)
    {
        if(this->file!=NULL)
        {
            this->load_data();
        }
        return this->file;
    }
    else
    {
        return NULL;
    }
}

nifti_image * niftiDynamicObject::getPtrNoData()
{
    if(this!=NULL)
    {
        return this->file;
    }
    else
    {
        return NULL;
    }
}

bool niftiDynamicObject::getLoadStatus()
{
    if(this!=NULL && this->file!=NULL)
    {
        if( this->file->data !=NULL )
        {
            return 1;
        }
    }
    return 0;
}

void niftiDynamicObject::load_data()
{
    if(this!=NULL && this->file!=NULL)
    {
        if(this->file->data==NULL)
        {
            nifti_image * tmp=nifti_image_read(this->filename.c_str(),true);
            nifti_free_extensions(tmp);
            if(tmp->data==NULL)
            {
                cerr << "ERROR:Could not read or allocate image "<< this->filename.c_str()<< endl;
                exit(1);
            }
            nifti_image_free(this->file);
            this->file=tmp;
            this->file->scl_inter=0;
            this->file->scl_slope=1;
            if(this->file->datatype!=NIFTI_TYPE_FLOAT32)
                seg_changeDatatype<float>(this->file);

            nifti_update_dims_from_array(this->file);
        }
    }
}

void niftiDynamicObject::unload_data()
{
    if(this!=NULL && this->file!=NULL)
    {
        if( this->file->data !=NULL )
        {
            free(this->file->data );
        }
        this->file->data=NULL;
    }
}

// Geodesic image object

geoObject::geoObject()
{
    this->file=NULL;
}

//Takes ownership of the data
geoObject::geoObject(nifti_image * nifti_img)
    : niftiObject(nifti_img)
{
    if(nifti_img!=NULL && nifti_img->fname!=NULL)
    {
        this->filename=nifti_img->fname;
    }
    else
    {
        this->filename="";
    }
    if(nifti_img!=NULL)
    {
        this->file=nifti_copy_nim_info(nifti_img);
        this->file->dim[0]=this->file->ndim=3;
        this->file->dim[4]=this->file->nt=1;
        this->file->dim[5]=this->file->nu=1;
        this->file->pixdim[5]=this->file->du=1;
        nifti_update_dims_from_array(this->file);
        this->file->datatype = NIFTI_TYPE_FLOAT32;
        this->file->nbyper = sizeof(float);
        this->file->data = (void *)malloc(this->file->nvox * this->file->nbyper);
        float * tmpptr = static_cast<float *>(this->file->data);

        for(size_t i=0; i<this->file->nvox ; i++)
        {
            tmpptr[i]=std::numeric_limits<float>::max();
        }
    }
    else
    {
        this->file=NULL;
    }
}


void geoObject::addIfSmaller(nifti_image * _input){

    if(_input!=NULL && (this->file->nx*this->file->ny*this->file->nz)==(_input->nx*_input->ny*_input->nz))
    {
        float * tmpPtrInput = static_cast<float *>(_input->data);
        float * tmpPtrObj = static_cast<float *>(this->file->data);
        for(size_t i=0; i<(this->file->nx*this->file->ny*this->file->nz) ; i++)
        {
            // if the input is nan at voxel I, do nothing
            if(isnan(tmpPtrInput[i])==0)
            {
                // if the input is not nan at voxel I, but the current object is, then always replace value;
                //if(isnan(tmpPtrObj[i])==std::numeric_limits<float>::max())
                tmpPtrObj[i]=tmpPtrObj[i]<tmpPtrInput[i]?tmpPtrObj[i]:tmpPtrInput[i];
            }
        }
    }
    else
    {
        cerr<<"ERROR: Input to the geodesic distance object is the wrong size."<<endl;
        exit(1);
    }


    return;
}


// MapImage

mapImage::mapImage(int _size)
{
    for(int i=0; i<_size; i++)
    {
        this->Data.push_back(new DataPointMap);
    }
}

mapImage::mapImage(int _size,int * maskImage)
{
    for(int i=0; i<_size; i++)
    {
        if(maskImage==NULL || maskImage[i]>0)
        {
            this->Data.push_back(new DataPointMap);
        }
        else
        {
            this->Data.push_back(NULL);
        }
    }
}

mapImage::~mapImage()
{
    for(unsigned int i=0; i<this->Data.size(); i++)
    {
        delete this->Data[i];
        this->Data[i]=NULL;
    }
}

void mapImage::addToPair(int index,
                         unsigned char label,
                         float value)
{
    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        std::map<unsigned char,float>::iterator location=CurrDataPoint->find(label);
        if(location!=CurrDataPoint->end())
        {
            location->second=location->second+value;
        }
        else
        {
            CurrDataPoint->insert(DataPointPair(label,value));
        }
    }
}

void mapImage::assignToPair(int index,
                            unsigned char label,
                            float value)
{
    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        std::map<unsigned char,float>::iterator location=CurrDataPoint->find(label);
        if(location!=CurrDataPoint->end())
        {
            // If value == 0, then clear the pair
            CurrDataPoint->erase(location);
        }
        CurrDataPoint->insert(DataPointPair(label,value));
    }
}

void mapImage::assignSortToPair(int index,
                                unsigned char maxItemsInSort,
                                float inputIntensity,
                                float inputDistance)
{

    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        // if it's empty, just add the value
        if(CurrDataPoint->empty()){
            CurrDataPoint->insert(DataPointPair(0,inputDistance));
            CurrDataPoint->insert(DataPointPair(maxItemsInSort,inputIntensity));

        }
        else{
            unsigned char currentNumberOfItemsInSort=(int)(floor(CurrDataPoint->size()/2.0f));
            // first check the largest Distance in the current sort (the last element).
            // If the input Distance is smaller than this value, then we actually need to do something.
            // Otherwise, just skip everything.
            std::map<unsigned char,float>::iterator initialTestLocation=CurrDataPoint->find(currentNumberOfItemsInSort-1);
            if(initialTestLocation!=CurrDataPoint->end() && (initialTestLocation->second > inputDistance || currentNumberOfItemsInSort>=maxItemsInSort)){

                // Create a temporary vector to store the distance and intensities
                float * distances = new float [maxItemsInSort+1];
                float * distancesSort = new float [maxItemsInSort+1];
                float * intensities = new float [maxItemsInSort+1];

                // Add the new input distance/intensity at the beginning of the vector
                distances[0]= inputDistance;
                distancesSort[0]= inputDistance;
                intensities[0]= inputIntensity;


                // Fetch and add the  distance/intensity of the other elements and count them.
                for(int label=0; label<std::min(maxItemsInSort,(unsigned char)(currentNumberOfItemsInSort)); label++)
                {
                    std::map<unsigned char,float>::iterator currentDistanceIterator=CurrDataPoint->find(label);
                    std::map<unsigned char,float>::iterator currentIntensityIterator=CurrDataPoint->find(label+maxItemsInSort);

                    // Collect all distances and intensities if they exist
                    if(currentDistanceIterator!=CurrDataPoint->end() && currentIntensityIterator!=CurrDataPoint->end())
                    {
                        distances[label+1]= currentDistanceIterator->second;
                        distancesSort[label+1]= currentDistanceIterator->second;
                        intensities[label+1]= currentIntensityIterator->second;
                    }
                    else{
                        break;
                    }
                }
                // Clear the current values in CurrDataPoint to replace by the new sorted ones
                CurrDataPoint->clear();

                // Actually sort the distancesSort vector and obtain the order vector.
                currentNumberOfItemsInSort=currentNumberOfItemsInSort+1;
                int * order=quickSort_order(distancesSort,currentNumberOfItemsInSort);

                // Write the first min(maxItemsInSort,numberOfElements) labels back into the empty CurrDataPoint
                for(int label=0; label<std::min(maxItemsInSort,(unsigned char)(currentNumberOfItemsInSort)); label++)
                {
                    CurrDataPoint->insert(DataPointPair(label,distances[order[label]]));
                    CurrDataPoint->insert(DataPointPair(label+maxItemsInSort,intensities[order[label]]));
                }

                delete [] distances;
                delete [] distancesSort;
                delete [] intensities;
                delete [] order;

            }
        }
    }
}

float mapImage::getValue(int index,
                         unsigned char label)
{
    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        std::map<unsigned char,float>::iterator it;
        if ( (it = CurrDataPoint->find(label)) != CurrDataPoint->end() )
        {
            return it->second;
        }
        else{
            return 0.0f;
        }
    }
    else
    {
        return std::numeric_limits<float>::quiet_NaN();
    }
}

float mapImage::getValueOrNaN(int index,
                              unsigned char label)
{
    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        std::map<unsigned char,float>::iterator it;
        if ( (it = CurrDataPoint->find(label)) != CurrDataPoint->end() )
        {
            return it->second;
        }
        else{
            return std::numeric_limits<float>::quiet_NaN();
        }
    }
    else
    {
        return std::numeric_limits<float>::quiet_NaN();
    }
}

unsigned char mapImage::getKeyWithMaxVal(int index)
{
    DataPointMap * CurrDataPoint=this->Data[index];
    if(CurrDataPoint!=NULL)
    {
        std::map<unsigned char,float>::iterator currIterator = CurrDataPoint->begin();
        int maxindex=0;
        float maxval=-std::numeric_limits<float>::max();;
        while(currIterator != CurrDataPoint->end())
        {
            if(currIterator->second>maxval)
            {
                maxindex=currIterator->first;
                maxval=currIterator->second;
            }
            currIterator++;
        }
        return maxindex;
    }
    else
    {
        return std::numeric_limits<unsigned char>::quiet_NaN();
    }
}

void mapImage::mask(int * maskImage)
{
    for(unsigned int i=0; i<this->Data.size(); i++)
    {
        if(maskImage[i]==0)
        {
            if(this->Data[i]!=NULL)
            {
                delete this->Data[i];
            }
            this->Data[i]=NULL;
        }
    }
}

// ************ Root Database Structure

Database::Database()
{
    this->path="";
    this->initialised=false;
}

void Database::init(string _folderpath,string target_filename)
{
    this->initialised=true;
    this->path=_folderpath.c_str();
    vector<string> current_filenames;
    DIR *dp;
    struct dirent64 *dirp;
    if((dp=opendir(_folderpath.c_str()))==NULL)
    {
        cerr << "Error(" << errno << ") opening " << _folderpath << endl;
        exit(1);
    }

    while ((dirp=readdir64(dp)) !=NULL)
    {
        if(dirp->d_name[0]!='.')
        {
#if defined(_DIRENT_HAVE_D_TYPE) || (defined(__APPLE__) && defined(__MACH__))
#if defined WIN32 && !defined __CYGWIN__
            if (dirp->d_type == DT_REG || dirp->d_type == DT_UNKNOWN)
#else
            if(dirp->d_type==8 || dirp->d_type==10 || dirp->d_type==0) // d_type==0 is only alowed because of an NFS filesystem bug
#endif
            {
#endif
                string curnamefile=string(dirp->d_name);

                string currentshortname=curnamefile.substr(curnamefile.find_last_of(SEP)+1,curnamefile.size()-curnamefile.find_last_of(SEP));
                currentshortname=currentshortname.substr(0,std::min(std::min(currentshortname.find(".nii",currentshortname.size()-7),currentshortname.find(".img",currentshortname.size()-7)),currentshortname.find(".hdr",currentshortname.size()-7)) );

                string curtargetname=target_filename.substr(target_filename.find_last_of(SEP)+1,target_filename.size()-target_filename.find_last_of(SEP));
                curtargetname=curtargetname.substr(0,std::min(std::min(curtargetname.find(".nii",curtargetname.size()-7),curtargetname.find(".img",curtargetname.size()-7)),curtargetname.find(".hdr",curtargetname.size()-7)) );

                if(currentshortname.compare(curtargetname)!=0){
                    string curstring;
                    curstring=_folderpath;
                    curstring.append(SEP);
                    curstring.append(dirp->d_name);
                    current_filenames.push_back(curstring);
                }
                else{
                    cout<<"Status: Ignoring file "<<curnamefile<<" from the database (same name as the target)"<<endl;
                }
#if defined(_DIRENT_HAVE_D_TYPE) || (defined(__APPLE__) && defined(__MACH__))
            }
#endif
        }
    }
    std::sort( current_filenames.begin(), current_filenames.end() );
    closedir(dp);


    for(unsigned int i=0; i<current_filenames.size(); i++)
    {
        this->datasets.push_back(smartPointer<niftiDynamicObject>(new niftiDynamicObject(current_filenames[i],false)));
        this->datasets[this->datasets.size()-1]->unload_data();
    }
    current_filenames.clear();
}

void Database::init(string _folderpath,
                    string _description,
                    string target_filename)
{
    this->initialised=true;
    this->path=_folderpath;
    this->description=_description;
    vector<string> current_filenames;
    vector<string> current_foldernames;
    get_all_files_and_folders_in_dir(_folderpath,current_filenames,current_foldernames);
    for(unsigned int i=0; i<current_filenames.size(); i++)
    {
        string currentshortname=current_filenames[i].substr(current_filenames[i].find_last_of(SEP)+1,current_filenames[i].size()-current_filenames[i].find_last_of(SEP));
        string curtargetshortname=target_filename.substr(target_filename.find_last_of(SEP)+1,target_filename.size()-target_filename.find_last_of(SEP));
        if(currentshortname.compare(curtargetshortname)!=0){
            smartPointer<niftiDynamicObject> tmp_dataset=new niftiDynamicObject (current_filenames[i],false);
            this->datasets.push_back(tmp_dataset);
            tmp_dataset->unload_data();
        }
        else{
            cout<<"Status: Skipping file "<<current_filenames[i]<<" from the db, as it has the same name as the target"<<endl;
        }
    }
}

void Database::setDescription(string _description)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    this->description=_description;
    return;
}

string Database::getDescription()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->description;
}

string Database::getPath()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->path;
}

size_t Database::getNumberOfDatasets()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->datasets.size();
}

nifti_image * Database::getDatasetPtr(unsigned int numb)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(numb<this->datasets.size() && this->datasets[numb].cPTR())
    {
        return this->datasets[numb]->getPtr();
    }
    else
    {
        return NULL;
    }
}

nifti_image * Database::getDatasetNoDataPtr(unsigned int numb)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(numb<this->datasets.size() && this->datasets[numb].cPTR())
    {
        return this->datasets[numb]->getPtrNoData();
    }
    else
    {
        return NULL;
    }
}



string Database::getDatasetName(unsigned int numb)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(numb<this->datasets.size())
    {
        return this->datasets[numb]->getFilename();
    }
    else
    {
        cout << "Could not return dataset "<<numb << " filename from path "<<this->path<<endl;
        exit(1);
    }
}

void Database::setDataset(unsigned int numb,
                          smartPointer<niftiDynamicObject> newDataset)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(numb<this->datasets.size())
    {
        this->datasets[numb]=newDataset;
    }
    else
    {
        cout << "Could not re-set dataset "<<numb << " from path "<<this->path<<endl;
        exit(1);
    }
}

void Database::unloadAllData(bool _verbose)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(_verbose>1)
    {
        cout << "# Unloading data from the root"<<endl;
    }
    for(size_t datasetIndex=0; datasetIndex<this->datasets.size(); datasetIndex++)
    {
        this->unloadDataFromDataset(datasetIndex);
    }

}

void Database::unloadDataFromDataset(unsigned int numb)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(numb<this->datasets.size())
    {
      if( this->datasets[numb].cPTR()!=NULL )
        this->datasets[numb]->unload_data();
      return;
    }
}


Database::~Database()
{
}

// ************ Database Structure

void infoDatabase::init(string _folderpath,
                        smartPointer<Database> rootdb,
                        string target_filename)
{
    this->initialised=true;
    this->path=_folderpath;
    this->type=-1;
    this->target_nifti=new niftiDynamicObject ();
    this->target_nifti_int_mask=NULL;

    // get root filenames
    vector<string> root_filenames;
    for(size_t i=0; i<rootdb->getNumberOfDatasets(); i++)
    {
        root_filenames.push_back(rootdb->getDatasetNoDataPtr(i)->fname);
        rootdb->unloadDataFromDataset(i);
    }


    // get database filenames
    vector<string> current_filenames;
    DIR *dp;
    struct dirent64 *dirp;
    dp=opendir(_folderpath.c_str());

    while (dp!=NULL && (dirp=readdir64(dp)) !=NULL)
    {
        if(dirp->d_name[0]!='.')
        {
#ifdef _DIRENT_HAVE_D_TYPE
#if defined WIN32 && !defined __CYGWIN__
      if (dirp->d_type == DT_REG || dirp->d_type == DT_UNKNOWN)
#else
          if(dirp->d_type==0 || dirp->d_type==8 || dirp->d_type==10)
#endif
            {
#endif
                string curstring;
                curstring=_folderpath;
                curstring.append(SEP);
                curstring.append(dirp->d_name);
                current_filenames.push_back(curstring);
            }
#ifdef _DIRENT_HAVE_D_TYPE
        }
#endif

    }
    if(dp!=NULL){
        std::sort(current_filenames.begin(), current_filenames.end() );
        closedir(dp);
    }

    for(unsigned int j=0; j<root_filenames.size(); j++)
    {
        int comparetrue=-1;
        //get basename of the root string
        if(dp!=NULL){
            string short_root_string=root_filenames[j].substr(root_filenames[j].find_last_of(SEP),root_filenames[j].size()-root_filenames[j].find_last_of(SEP));
            for(unsigned int i=0; i<current_filenames.size(); i++)
            {
                //get basename of the current string
                string short_current_string=current_filenames[i].substr(current_filenames[i].find_last_of(SEP),current_filenames[i].size()-current_filenames[i].find_last_of(SEP));
                //test if same
                string curtargetname=target_filename.substr(target_filename.find_last_of(SEP)+1,target_filename.size()-target_filename.find_last_of(SEP));
                if(short_root_string.compare(short_current_string)==0 && curtargetname.compare(short_current_string)!=0)
                {
                    comparetrue=i;
                }
            }
        }
        // if they are the same
        if(dp!=NULL && comparetrue>=0)
        {
            smartPointer<niftiDynamicObject> tmp = new niftiDynamicObject(current_filenames[comparetrue],false);
            this->datasets.push_back(tmp);
#ifndef NDEBUG
            cout<<"Found matching pair: "<<root_filenames[j]<< " - "<<current_filenames[comparetrue]<<endl;
#endif
            this->datasets[this->datasets.size()-1]->unload_data();
        }
        else
        {
            this->datasets.push_back(NULL);
        }
    }
}

//create empty database
void infoDatabase::init(smartPointer<Database> rootdb)
{
    this->initialised=true;
    this->path=".";
    //srand(time(NULL));
    //this->fname=rand();
    this->fname="";
    this->type=-1;
    this->target_nifti=new niftiDynamicObject ();
    this->target_nifti_int_mask=NULL;
    // get root filenames
    vector<string> root_filenames;
    for(size_t i=0; i<rootdb->getNumberOfDatasets(); i++)
    {
        root_filenames.push_back(rootdb->getDatasetNoDataPtr(i)->fname);
        rootdb->unloadDataFromDataset(i);
    }

    for(size_t j=0; j<root_filenames.size(); j++)
    {
        this->datasets.push_back(NULL);
    }
}

infoDatabase::~infoDatabase()
{
    this->path.clear();
    this->description.clear();

    // if mask was allocated, then delete it
    if(this->target_nifti_int_mask!=NULL)
    {
        delete [] this->target_nifti_int_mask;
    }
}


int * infoDatabase::getMaskPtr()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(this->target_nifti_int_mask!=NULL)
    {
        return this->target_nifti_int_mask;
    }
    else
    {
        return NULL;
    }
}

nifti_image * infoDatabase::getTargetPtr()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->target_nifti->getPtr();
}

void infoDatabase::setMaskPtr(int * _mask_PTR)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(this->target_nifti_int_mask==NULL)
    {
        this->target_nifti_int_mask=_mask_PTR;
    }
    else
    {
        delete [] this->target_nifti_int_mask;
        this->target_nifti_int_mask=_mask_PTR;
    }
    return;
}

void infoDatabase::setTarget(string _target_filename)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    this->target_nifti= new niftiDynamicObject(_target_filename.c_str(),false);
    return;
}

void infoDatabase::unloadTarget()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    if(this->target_nifti->getLoadStatus())
    {
        this->target_nifti->unload_data();
        return;
    }
}

void infoDatabase::setType(string _type)
{
    if(_type=="Label")
    {
        this->type=LABEL;
        return;
    }
    else if(_type=="Probability")
    {
        this->type=PROB;
        return;
    }
    if(_type=="LabelSeg")
    {
        this->type=LABELSEG;
        return;
    }
    else if(_type=="ProbabilitySeg")
    {
        this->type=PROBSEG;
        return;
    }
    else if(_type=="Intensity")
    {
        this->type=INTEN;
        return;
    }
    else if(_type=="IntensitySort")
    {
        this->type=INTENSORT;
        return;
    }
    else if(_type=="IntensityMAP")
    {
        this->type=INTENMAP;
        return;
    }
    else if(_type=="Likelihood")
    {
        this->type=LIKE;
        return;
    }
    else if(_type=="Velocity")
    {
        this->type=VEL;
        return;
    }
    else
    {
        cout<<"Invalid Database Type: "<< _type;
        exit(1);
    }
}

unsigned int infoDatabase::getType()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->type;
}

void infoDatabase::setFname(string _fname)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    this->fname=_fname;
}

string infoDatabase::getFname()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->fname;
}

void infoDatabase::setExtraFiles(vector<string> _extrafiles)
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    this->extrafiles=_extrafiles;
}

vector<string> infoDatabase::getExtraFiles()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return this->extrafiles;
}

int infoDatabase::getNt()
{
    if(this->initialised==false)
    {
        cerr<< "ERROR: Database has not been initialised"<< endl;
        exit(1);
    }
    return atoi(this->nt.c_str());
}

// ************ GIF structure

GIF::GIF(string _xmlFilename,
         string _target_filename )
{

    this->target=new niftiDynamicObject(_target_filename.c_str(),true);
    this->targetLaplacian=new niftiDynamicObject(NULL,1);
    this->targetMask=new int [this->target->getPtrNoData()->nvox];
    for(size_t index=0; index<(size_t)(this->target->getPtrNoData()->nvox); index++)
        this->targetMask[index]=1;

    this->groupMask=new niftiDynamicObject(NULL,1);;


    this->sFormAligned=0;
    this->inputAffine="";
    this->saveTransformations=false;
    this->verbose=0;
    this->ompj=1;

    this->kernelstdDef=-2.0f;
    this->kernelstdSsd=-2.0f;
    this->kernelstdLncc=-2.0f;

    this->weightLSSD=0.0f;
    this->weightDEF=0.0f;
    this->weightLNCC=1.0f;
    this->weightT1DTI=0.0f;
    this->nrr_metric=0;

    this->temperature=0.15f;

    this->sortMaxItems=7;
    this->sortBeta=0.5;

    this->updateLab=0;
    this->saveGeo=0;

    this->regRunSecondLevel=true;
    this->regBendingEnergyWeight=0.005;
    this->regJacobianLogWeight=0.0001;
    this->regSpeed=1;

    this->segRFsmo=0;
    this->segRFalpha=0;
    this->segMRFbeta=0.15;
    this->segMinNumIter=3;


    this->DatabaseToTargetAffine=NULL;
    if(_xmlFilename.find_last_of(SEP)!=string::npos)
    {
        this->basePath=_xmlFilename.substr(0,_xmlFilename.find_last_of(SEP));
    }
    else
    {
        this->basePath=string("./");
    }
    this->xmlFileCharPtr=xmlReadFile(_xmlFilename);
    //cout<<this->xmlFileCharPtr<<endl;
    this->xml=xmlReadFromCharPtr(xmlFileCharPtr);
    this->resultsPath=".";

    for (xml_node<> *current_node=this->xml->first_node()->first_node(); current_node; current_node=current_node->next_sibling())
    {
        if(strcmp(current_node->name(),"data")==0)
        {
            string curDescr="";
            bool exists=false;
            // Find path and create database
            curDescr=xmlGetTextFromXmlNode(current_node,(char*)"path",&exists);
            if(exists==false)
            {
                cerr << "ERROR: Mandatory field 'path' does not exist."<<endl;
                exit(1);
            }
            else
            {
                this->rootdb=new Database();
                this->rootdb->init(this->basePath+SEP+curDescr, _target_filename);
            }

            // Find and Set Description field
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"descr",&exists);
            if(exists==false)
            {
                cout << "Status: Field 'descr' does not exist. Setting to Rootdb."<<endl;
                this->rootdb->setDescription(string("Rootdb"));
            }
            else
            {
                this->rootdb->setDescription(curDescr);
            }

            // Find and Set Sform field
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"sform",&exists);
            if(exists==false)
            {
                cout << "Status: Field 'sform' does not exist. Assuming 0."<<endl;
                this->setSformAlignedDatabase(0);
            }
            else
            {
                this->setSformAlignedDatabase((char)(atoi(curDescr.c_str())));
            }


            // Find and Set the GroupMask if it exists
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"gm",&exists);
            if(exists==true && curDescr.size()>0)
            {
                this->setRoiMask(this->basePath+SEP+curDescr);
            }

            // If a <data> element is found and parsed, then finish loop
            current_node=current_node->last_node();
        }
    }
    this->freeMemory();
    for (xml_node<> *current_node=this->xml->first_node()->first_node(); current_node; current_node=current_node->next_sibling())
    {
        if(strcmp(current_node->name(),"info")==0)
        {
            string curDescr="";
            bool exists=false;
            // Find path and create info database
            curDescr=xmlGetTextFromXmlNode(current_node,(char*)"path",&exists);
            if(exists==false)
            {
                cerr << "ERROR: Mandatory field 'path' does not exist."<<endl;
                exit(1);
            }
            else
            {
                this->infoDB.push_back(new infoDatabase());
                this->infoDB[this->infoDB.size()-1]->init(this->basePath+SEP+curDescr,this->rootdb,_target_filename);
            }

            // Find path and create GEO database
            curDescr=xmlGetTextFromXmlNode(current_node,(char*)"gpath",&exists);
            if(exists==false)
            {
                //cout << "Status: Field 'gpath' does not exist. Setting to empty."<<endl;
                curDescr=xmlGetTextFromXmlNode(current_node,(char*)"path",&exists);
                this->geoDB.push_back(new infoDatabase());
                this->geoDB[this->geoDB.size()-1]->init(this->rootdb);
            }
            else
            {
                this->geoDB.push_back(new infoDatabase());
                if(curDescr.size()>0)
                {
                    this->geoDB[this->geoDB.size()-1]->init(this->basePath+SEP+curDescr,this->rootdb,_target_filename);
                }
                else
                {
                    this->geoDB[this->geoDB.size()-1]->init(this->rootdb);
                }
            }

            // Find and Set Type field
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"type",&exists);
            if(exists==false)
            {
                cerr << "ERROR: Mandatory field 'type' does not exist."<<endl;
                exit(1);
            }
            else
            {
                this->infoDB[this->infoDB.size()-1]->setType(curDescr);
            }

            // Find and Set fname field
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"fname",&exists);
            if(exists==false)
            {
                cerr << "ERROR: Mandatory field 'fname' does not exist."<<endl;
                exit(1);
            }
            else
            {
                this->infoDB[this->infoDB.size()-1]->setFname(curDescr);
            }

            // Find and Set Description field
            curDescr=xmlGetTextFromXmlNode(current_node,(char *)"descr",&exists);
            if(exists==false)
            {
                cout << "Status: Field 'descr' does not exist. Setting to 'Info'"<<endl;
                curDescr=xmlGetTextFromXmlNode(current_node,(char*)"path",&exists);
                this->infoDB[this->infoDB.size()-1]->setDescription(string("Info"));
            }
            else
            {
                this->infoDB[this->infoDB.size()-1]->setDescription(curDescr);
            }

            // Find and Set extra fields 1 2 and 3
            bool exists1;
            string curDescr1=xmlGetTextFromXmlNode(current_node,(char *)"extra",&exists1);
            if(exists1==false)curDescr1=string("");

            bool exists2;
            string curDescr2=xmlGetTextFromXmlNode(current_node,(char *)"extra2",&exists2);
            if(exists2==false)curDescr2=string("");

            bool exists3;
            string curDescr3=xmlGetTextFromXmlNode(current_node,(char *)"extra3",&exists3);
            if(exists3==false)curDescr3=string("");
            vector<string> curDescrExtras;

            curDescrExtras.push_back(curDescr1);
            curDescrExtras.push_back(curDescr2);
            curDescrExtras.push_back(curDescr3);

            this->infoDB[this->infoDB.size()-1]->setExtraFiles(curDescrExtras);
            this->freeMemory();

        }
    }
    // Create empty database for the CPP's
    this->cppdb=new infoDatabase();
    this->cppdb->init(this->rootdb);
    this->cppdb->setFname("cpp");

    for(int item=0; item<this->rootdb.cPTR()->getNumberOfDatasets(); item++)
       this->affDB.push_back(new mat44);
}

void GIF::allocateInfoGeoResStuctures()
{

    for(size_t databasenumb=0; databasenumb<this->infoDB.size(); databasenumb++)
    {
        if(this->infoDB[databasenumb]->getTargetPtr()==NULL)
        {
            this->resultsInfoDB.push_back(new mapImage(this->target->getPtr()->nx*this->target->getPtr()->ny*this->target->getPtr()->nz,this->targetMask));
            this->resultsGeoDB.push_back(new geoObject(this->target->getPtr()));
        }
        else
        {
            this->resultsInfoDB.push_back(new mapImage(this->infoDB[databasenumb]->getTargetPtr()->nx*this->infoDB[databasenumb]->getTargetPtr()->ny*this->infoDB[databasenumb]->getTargetPtr()->nz,this->infoDB[databasenumb]->getMaskPtr()));
            this->resultsGeoDB.push_back(new geoObject(this->infoDB[databasenumb]->getTargetPtr()));
        }

    }
    return;
}

void GIF::freeMemory()
{

    this->rootdb->unloadAllData(this->getVerbose());
    for(size_t databasenumb=0; databasenumb<this->infoDB.size(); databasenumb++)
        this->infoDB[databasenumb]->unloadAllData(this->getVerbose());

}


GIF::~GIF()
{
    delete [] this->xmlFileCharPtr;

    if(this->targetMask!=NULL)
    {
        delete [] this->targetMask;
    }
    resultsInfoDB.clear();
    resultsGeoDB.clear();
    infoDB.clear();
    geoDB.clear();
    affDB.clear();
}
vector< vector<int> >  GIF::readLabMappingXml(string _filename,
                                              unsigned char _maxsize)
{

    char * CoordinateFileCharPtr=xmlReadFile(_filename);
    xml_document<> * baseXMLDocument=xmlReadFromCharPtr(CoordinateFileCharPtr);
    vector< vector<int> > mapping(_maxsize,vector<int>(0));


    // Find <document> node
    rapidxml::xml_node<> *document_node=NULL;
    for (rapidxml::xml_node<> *base_node=baseXMLDocument->first_node(); base_node; base_node=base_node->next_sibling())
    {
      if(strcmp(base_node->name(),"document")==0)
      {
        document_node=base_node;
      }
    }
    if(document_node==NULL){fprintf(stderr,"* Error: <document> node not found in XML file\n"); return mapping;}

    // Find <labels> <tissues> node
    rapidxml::xml_node<> *labels_node=NULL;
    rapidxml::xml_node<> *tissues_node=NULL;
    for (rapidxml::xml_node<> *base_node=document_node->first_node(); base_node; base_node=base_node->next_sibling())
    {
      if(strcmp(base_node->name(),"labels")==0)
      {
        labels_node=base_node;
      }
      if(strcmp(base_node->name(),"tissues")==0)
      {
        tissues_node=base_node;
      }
    }
    if(labels_node==NULL){fprintf(stderr,"* Error: <labels> node not found inside the <document> node\n"); return mapping;}
    if(tissues_node==NULL){fprintf(stderr,"* Error: <tissues> node not found inside the <document> node\n"); return mapping;}



    for (xml_node<> *current_node=labels_node->first_node(); current_node; current_node=current_node->next_sibling())
    {
        if(strcmp(current_node->name(),"item")==0)
        {
            // Label Number
            bool exists=false;
            int label=atoi(xmlGetTextFromXmlNode(current_node,(char*)"number",&exists).c_str());
            string tissuesstr=xmlGetTextFromXmlNode(current_node,(char*)"tissues",&exists);
            std::istringstream ss(tissuesstr);
            std::string token;
            while(std::getline(ss, token, ','))
            {
                int token_int=atoi(token.c_str());
                if(label<_maxsize)
                {

                    mapping[label].push_back(token_int);
                }
            }


        }
    }

    delete baseXMLDocument;
    delete [] CoordinateFileCharPtr;
    return mapping;
}


void GIF::setTransformationsFolder(string _target_folder )
{

    string target_path=_target_folder+SEP;

    DIR *dp;
    if((dp=opendir(target_path.c_str()))==NULL)
    {
        cerr << "ERROR: Could not open the cpp path -cpp (" << target_path <<"). Please create the folder first."<< endl;
        exit(1);
    }
    closedir(dp);

    this->saveTransformations=true;
    this->cppdb=new infoDatabase();
    this->cppdb->init(_target_folder,this->rootdb,this->target->getFilename());
    this->cppdb->setFname("cpp");
}

void GIF::setRootTargetMask(string _target_filename)
{

    smartPointer<niftiDynamicObject> tmp=new niftiDynamicObject(_target_filename.c_str(),true);
    float * tmpPtr=static_cast<float *>(tmp->getPtr()->data);

    if(this->targetMask==NULL)
    {
        this->targetMask=new int [this->target->getPtrNoData()->nx*this->target->getPtrNoData()->ny*this->target->getPtrNoData()->nz];
    }

    for(size_t index=0; index<(size_t)(this->target->getPtrNoData()->nx*this->target->getPtrNoData()->ny*this->target->getPtrNoData()->nz); index++)
    {
        this->targetMask[index]=(int)(tmpPtr[index]>0.0f);
    }
    return;
}

void GIF::setDatabaseTarget(string _database_fname,
                            string _target_filename)
{

    bool databaseFound=false;
    for(size_t databasenumb=0; databasenumb<this->infoDB.size(); databasenumb++)
    {
        if(this->infoDB[databasenumb]->getFname()==_database_fname)
        {
            databaseFound=true;
            this->infoDB[databasenumb]->setTarget(_target_filename);
        }

    }
    if(databaseFound==false)
    {
        cout<<"Could not find database "<<_database_fname<< " to add mask "<<_target_filename<<endl;
        exit(1);
    }
    return;
}

void GIF::setDatabaseTargetMask(string _database_fname,
                                string _target_filename)
{

    bool databaseFound=false;
    for(size_t databasenumb=0; databasenumb<this->infoDB.size(); databasenumb++)
    {
        if(this->infoDB[databasenumb]->getFname()==_database_fname)
        {
            if(this->infoDB[databasenumb]->getTargetPtr()==NULL)
            {
                cout<<"Can only set mask in database "<<_database_fname<< " if image is already set"<<endl;
                exit(1);
            }

            databaseFound=true;
            nifti_image * tmp_target=this->infoDB[databasenumb]->getTargetPtr();

            smartPointer<niftiDynamicObject> tmp_mask=new niftiDynamicObject(_target_filename.c_str(),true);

            if((tmp_mask->getPtr()->nx*tmp_mask->getPtr()->ny*tmp_mask->getPtr()->nz)!=(tmp_target->nx*tmp_target->ny*tmp_target->nz))
            {
                cout<<"The mask mask "<<_target_filename<< " is not the same size as teh corresponding target image"<<endl;
                exit(1);
            }

            float * tmp_mask_Ptr=static_cast<float *>(tmp_mask->getPtr()->data);
            int * tmp_mask_int_PTR=new int [(tmp_target->nx*tmp_target->ny*tmp_target->nz)];

            for(size_t index=0; index<(size_t)(tmp_target->nx*tmp_target->ny*tmp_target->nz); index++)
            {
                tmp_mask_int_PTR[index]=tmp_mask_Ptr[index]>0;
            }

            this->infoDB[databasenumb]->setMaskPtr(tmp_mask_int_PTR);

            // do not delete _mask_PTR, as it will be owned by infodb
            // do not delete tmpPtr as it is just pointing to the tmp smartPointer data

        }

    }
    if(databaseFound==false)
    {
        cout<<"Could not find database "<<_database_fname<< " to add mask "<<_target_filename<<endl;
        exit(1);
    }
    return;
}

char GIF::getSformAlignedDatabase()
{
    return this->sFormAligned;
}

void GIF::setRoiMask(string _filename)
{
    if(this->getVerbose()>1){
        cout<<"Loading file "<<_filename<<endl;
    }
    this->groupMask=new niftiDynamicObject(_filename,true);
}

void GIF::setSformAlignedDatabase(char _sform_aligned)
{
    this->sFormAligned=_sform_aligned;
}

void GIF::setInputAffineFromDatabase(string _input_affine)
{
    this->inputAffine=_input_affine;
}

void GIF::setResultsPath(string _path)
{
    string res_path=_path+SEP;

    DIR *dp;
    if((dp=opendir(res_path.c_str()))==NULL)
    {
        cerr << "ERROR: Could not open the output path -out (" << res_path <<"). Please create the folder first."<< endl;
        exit(1);
    }
    closedir(dp);


    FILE *testfile;
    string testFilename=_path+SEP+"test";
    if((bool)(testfile=fopen(testFilename.c_str(), "w")))
    {
        fclose(testfile);
        // remove test file after closing the stream.
        remove(testFilename.c_str());
    }
    else
    {
        cerr<< "ERROR: Cannot write to output path -out (" << res_path <<"). Please check the path's r-w permissions and disk space availability."<<endl;
        exit(1);
    }

    this->resultsPath=res_path;
}

void GIF::setVerbose(char _verbose)
{
    this->verbose=_verbose>2?2:(_verbose<0?0:_verbose);
}

void GIF::set_kernelstd_def(float _value)
{
    this->kernelstdDef=_value;
}

void GIF::set_kernelstd_ssd(float _value)
{
    this->kernelstdSsd=_value;
}

void GIF::set_kernelstd_lncc(float _value)
{
    this->kernelstdLncc=_value;
}

void GIF::set_kernelstd_t1dti(float _value)
{
    this->kernelstdT1DTI=_value;
}

void GIF::set_weight_def(float _value)
{
    this->weightDEF=_value<0?0:_value;
}

void GIF::set_weight_ssd(float _value)
{
    this->weightLSSD=_value<0?0:_value;
}

void GIF::set_weight_lncc(float _value)
{
    this->weightLNCC=_value<0?0:_value;
}

void GIF::set_weight_t1dti(float _value)
{
    this->weightT1DTI=_value<0?0:_value;
}


void GIF::set_temperature(float _value)
{
    this->temperature=_value<0?0:_value;
}

void GIF::set_sort_number(char _value)
{
    this->sortMaxItems=_value<1?1:_value;
}

void GIF::set_sort_beta(float _value)
{
    this->sortBeta=_value<0.01?0.01:_value;
}

void GIF::set_nrr_metric(char _value)
{
    // 1 for NMI, 0 for LNCC
    this->nrr_metric=_value>0?1:0;
}

void GIF::set_regRunSecondLevel(bool _value)
{
    // 1 for NMI, 0 for LNCC
    this->regRunSecondLevel=_value;
}

void GIF::set_regBendingEnergyWeight(float _value)
{
    // 1 for NMI, 0 for LNCC
    this->regBendingEnergyWeight=_value>0?_value:0;
}

void GIF::set_regJacobianLogWeight(float _value)
{
    this->regJacobianLogWeight=_value>0?_value:0;
}

void GIF::set_segRFsmo(float _value)
{
    this->segRFsmo=_value>0?_value:0;
}

void GIF::set_segRFalpha(float _value)
{
    this->segRFalpha=_value>0?(_value<1?_value:1):0;
}

void GIF::set_regSpeed(int _value)
{
    // 1 for no speed-up, 2 for speed-up
    this->regSpeed=(int)_value+1;
}

void GIF::set_segMRFbeta(float _value)
{
    this->segMRFbeta=_value>0?_value:0;
}

void GIF::set_segMinNumIter(float _value)
{
    this->segMinNumIter=_value>1?_value:1;
}


void GIF::set_updateLab(bool _value){
    this->updateLab=_value;
}

void GIF::set_saveGeo(bool _value){
    this->saveGeo=_value;
}

void GIF::set_ompj(int _ompj){
    this->ompj=_ompj;
#ifdef _OPENMP
    if(this->ompj>1){
        omp_set_nested(true);
    }else{
        omp_set_nested(false);
    }
#endif
}




bool GIF::getAndSaveTransformations()
{
    if(this->cppdb->getPath().size()==0)
    {
        return 0;
    }
    else
    {
        return this->saveTransformations;
    }
}

char GIF::getVerbose()
{
    return this->verbose;
}



void GIF::Run()
{

#ifdef _OPENMP
    if(this->getVerbose()>0)
        cout<< "Status: OpenMP running with "<<omp_get_max_threads()<< " threads"<<endl;
#endif

    // Run Affine registrations in order to get the txt
    this->RegisterAllSubjectsAndGetAffine();

    // Run NRR registrations in order to get the cpps
    this->RegisterAllSubjectsAndGetCPP();

    // Allocate results data structure
    this->allocateInfoGeoResStuctures();

    // Calculate similarities and flow the information
    this->PropagateAllSubjectsInformation();

    // Clear the data before saving;
    this->freeMemory();
    this->cppdb->unloadAllData(this->getVerbose());


    // Save results
    this->processAndSaveResults();

}

void GIF::processAndSaveResults()
{
    if(this->getVerbose()>0)
    {
        cout<<"\n___________________________\n"<<endl;
        cout << "Saving Results"<<endl;
    }

    for(size_t databasenumb=0; databasenumb<this->resultsInfoDB.size(); databasenumb++)
    {
        this->processAndSaveResultingFlow(databasenumb);
    }
}

void GIF::processAndSaveResultingFlow(unsigned int databasenumb)
{

    switch(this->infoDB[databasenumb]->getType())
    {
    case LABEL:
        this->processAndSaveFlowAsLabel(databasenumb);
        break;
    case PROB:
        this->processAndSaveFlowAsProb(databasenumb);
        break;
    case LABELSEG:
        this->processAndSaveFlowAsLabelSeg(databasenumb);
        break;
    case PROBSEG:
        this->processAndSaveFlowAsProbSeg(databasenumb);
        break;
    case INTEN:
        this->processAndSaveFlowAsInten(databasenumb);
        break;
    case INTENSORT:
        this->processAndSaveFlowAsIntenSort(databasenumb);
        break;
    case INTENMAP:
        this->processAndSaveFlowAsIntenMAP(databasenumb);
        break;
    case LIKE:
        this->processAndSaveFlowAsLike(databasenumb);
        break;
    case VEL:
        this->processAndSaveFlowAsVel(databasenumb);
        break;
    default:
        cout<<"Invalid Database Type: "<< this->infoDB[databasenumb]->getType();
        exit(1);
        break;
    }
    this->resultsInfoDB[databasenumb]=NULL;
}

void GIF::processAndSaveFlowAsLabel(unsigned int databasenumb)
{
    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Allocate memory and resample floating into the target space
    niftiObject ResultingLabels(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    ResultingLabels.getPtr()->dim[0]=ResultingLabels.getPtr()->ndim=3;
    ResultingLabels.getPtr()->dim[4]=ResultingLabels.getPtr()->nt=1;
    ResultingLabels.getPtr()->dim[5]=ResultingLabels.getPtr()->nu=1;
    ResultingLabels.getPtr()->pixdim[5]=ResultingLabels.getPtr()->du=1;
    nifti_update_dims_from_array(ResultingLabels.getPtr());
    ResultingLabels.getPtr()->datatype = NIFTI_TYPE_FLOAT32;
    ResultingLabels.getPtr()->nbyper = sizeof(float);
    ResultingLabels.getPtr()->data = (void *)malloc(ResultingLabels.getPtr()->nvox * ResultingLabels.getPtr()->nbyper);
    if(ResultingLabels.getPtr()->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameResStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";

    if(this->getVerbose()>0)
        cout << "    - Saving Label Information: "<<filenameResStr<< endl;

    ResultingLabels.setFilename(filenameResStr);
    float * ResultingLabelsPtr=static_cast<float *>(ResultingLabels.getPtr()->data);
    size_t volnvox=ResultingLabels.getPtr()->nx*ResultingLabels.getPtr()->ny*ResultingLabels.getPtr()->nz;

    string lastLabelName;
    string lastGeoName;
    smartPointer<niftiDynamicObject> lastLabelImg;
    smartPointer<niftiDynamicObject> lastGeoImg;



    if( this->updateLab==0 ){// Normal run... Just get the ML estimate
        for(size_t index=0; index<volnvox; index++)
        {
            ResultingLabelsPtr[index]=this->resultsInfoDB[databasenumb]->getKeyWithMaxVal(index);
        }
    }
    else{ //Update the old label
        if(this->geoDB[databasenumb].cPTR()!=NULL){ // If the geoDB database exists, define the names and try to read the images
            string curfilename=this->target->getFilename();
            if(curfilename.find_last_of(SEP)>0)
            {
                curfilename=curfilename.substr(curfilename.find_last_of(SEP)+1,curfilename.size()-curfilename.find_last_of(SEP));
            }
            lastLabelName=this->infoDB[databasenumb]->getPath()+SEP+curfilename;
            lastGeoName=this->geoDB[databasenumb]->getPath()+SEP+curfilename;
        }
        FILE *lastLabFile;
        FILE *lastGeoFile;
        if((bool)(lastLabFile=fopen(lastLabelName.c_str(), "r")) && (bool)(lastGeoFile=fopen(lastGeoName.c_str(), "r"))){
            fclose(lastLabFile); fclose(lastGeoFile);
            cout<<"    - Updating Label assuming:"<<"\n        Last Label Name   - "<<lastLabelName<<"\n        Last GeoDist Name - "<<lastGeoName<<endl;
            lastLabelImg=smartPointer<niftiDynamicObject>(new niftiDynamicObject(lastLabelName,true));
            lastGeoImg=smartPointer<niftiDynamicObject>(new niftiDynamicObject(lastGeoName,true));

            // if the files exist, then only update the Labels where the current geo is smaller than the previous geo
            float * lastLabPtr=static_cast<float *>(lastLabelImg->getPtr()->data);
            float * lastGeoPtr=static_cast<float *>(lastGeoImg->getPtr()->data);
            float * currGeoPtr=static_cast<float *>(this->resultsGeoDB[databasenumb]->getPtr()->data);
            for(long index=0; index<volnvox; index++) // conditional update of the labels
            {
                ResultingLabelsPtr[index]=currGeoPtr[index]<lastGeoPtr[index]?this->resultsInfoDB[databasenumb]->getKeyWithMaxVal(index):lastLabPtr[index];
            }
            if(this->saveGeo){
                for(long index=0; index<volnvox; index++) // conditional update of the geos
                {
                    currGeoPtr[index]=currGeoPtr[index]<lastGeoPtr[index]?currGeoPtr[index]:lastGeoPtr[index];
                }
            }
        }
        else{ // If it could not find the image, just run normally
            cout<<"    - Could not find label to update. Running normally."<<endl;
            for(long index=0; index<volnvox; index++)
            {
                ResultingLabelsPtr[index]=this->resultsInfoDB[databasenumb]->getKeyWithMaxVal(index);
            }
        }
    }


    float min=FLT_MAX;
    float max=-FLT_MAX;
    for(long i=0; i<(long)(volnvox); i++)
    {
        max=ResultingLabelsPtr[i]>max?ResultingLabelsPtr[i]:max;
        min=ResultingLabelsPtr[i]<min?ResultingLabelsPtr[i]:min;
    }
    ResultingLabels.getPtr()->cal_min=min;
    ResultingLabels.getPtr()->cal_max=max;
    nifti_image_write(ResultingLabels.getPtr());

    if(this->saveGeo){
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving Label Geo Information: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }

}

void GIF::processAndSaveFlowAsLabelSeg(unsigned int databasenumb)
{

    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";

    if(this->getVerbose()>0)
        cout << "    - Segmenting using " << this->infoDB[databasenumb]->getFname() << " (mod="<< this->target->getPtr()->nt << ")" << endl;

    string xml_file_name=this->basePath+SEP+this->infoDB[databasenumb]->getExtraFiles()[0];
    vector< vector<int> > mapping = this->readLabMappingXml(xml_file_name,std::numeric_limits<unsigned char>::max());
    char maximumNumbClass=0;
    for(int i=0; i<255; i++)
    {
        for(int j=0; j<mapping[i].size(); j++)
        {
            maximumNumbClass=mapping[i][j]>maximumNumbClass?mapping[i][j]:maximumNumbClass;
        }
    }
    maximumNumbClass=maximumNumbClass+1;

    // Allocate space for segmentation prior
    smartPointer<niftiObject> ResultingProbImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    nifti_image * ResultingProb=ResultingProbImg->getPtr();
    ResultingProb->dim[0]=ResultingProb->ndim=4;
    ResultingProb->dim[4]=ResultingProb->nt=maximumNumbClass;
    ResultingProb->dim[5]=ResultingProb->nu=0;
    ResultingProb->pixdim[5]=ResultingProb->du=0;
    nifti_update_dims_from_array(ResultingProb);
    ResultingProb->datatype = NIFTI_TYPE_FLOAT32;
    ResultingProb->nbyper = sizeof(float);
    ResultingProb->data = (void *)malloc(ResultingProb->nvox * ResultingProb->nbyper);
    if(ResultingProb->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }
    float * ResultingProbPtr=static_cast<float *>(ResultingProb->data);

    filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_prior.nii.gz";

    nifti_set_filenames(ResultingProb,filenameStr.c_str(),0,0);

    // Generating tissue prior from the mapping function

    // Allocate nifti for mask
    smartPointer<niftiObject> MaskNiiImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    nifti_image *MaskNii = MaskNiiImg->getPtr();
    MaskNii->dim[0]=MaskNii->ndim=4;
    MaskNii->dim[4]=MaskNii->nt=1;
    MaskNii->dim[5]=MaskNii->nu=0;
    nifti_update_dims_from_array(MaskNii);
    MaskNii->datatype = NIFTI_TYPE_FLOAT32;
    MaskNii->nbyper = sizeof(float);
    MaskNii->data = (void *)malloc(MaskNii->nvox * MaskNii->nbyper);
    if(MaskNii->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate MaskNii"<< endl;
        exit(1);
    }
    float * MaskNiiPtr=static_cast<float *>(MaskNii->data);

    for(size_t index=0; index<MaskNii->nvox; index++)
    {
        if(this->targetMask[index])
        {
            MaskNiiPtr[index]=0;
            for(int nlab=0; nlab<maximumNumbClass; nlab++)
            {
                ResultingProbPtr[index+nlab*MaskNii->nvox]=0;
            }
            // Map probabilities to the tissue segmentation
            // For every parcelation in the image
            for(int nlab=0; nlab<std::numeric_limits<unsigned char>::max(); nlab++)
            {
                // Get the parcelation's probability
                float curprob=this->resultsInfoDB[databasenumb]->getValue(index,nlab);
                // For each tissue mapping[nlab][nclas] that belongs to parcelation [nlab]
                for(int nclas=0; nclas<mapping[nlab].size(); nclas++){
                    // Devide the probability of the parcelation nlab by the number of tissues it can belong to
                    ResultingProbPtr[index+mapping[nlab][nclas]*MaskNii->nvox]+=curprob/float(mapping[nlab].size());
                    // This is problematic with other things rather than Neuromorphometrics,
                    // as we are assuming that lonly labels above 1 will be part of the brain
                    if(mapping[nlab][nclas]>4){
                        MaskNiiPtr[index]+=curprob/float(mapping[nlab].size());
                    }
                }
            }

            // Estimate normalisation
            float normalized_weight=0;
            for(int nlab=0; nlab<maximumNumbClass; nlab++)
            {

                normalized_weight+=ResultingProbPtr[index+nlab*MaskNii->nvox];
            }
            // normalise per probabilistic channel
            if(normalized_weight<=0 || normalized_weight!=normalized_weight)
            {
                ResultingProbPtr[index]=1;
                MaskNiiPtr[index]=0;

                for(int nlab=1; nlab<maximumNumbClass; nlab++)
                {
                    ResultingProbPtr[index+nlab*MaskNii->nvox]=0;
                }
            }
            else
            {

                for(int nlab=0; nlab<maximumNumbClass; nlab++)
                {
                    ResultingProbPtr[index+nlab*MaskNii->nvox]=ResultingProbPtr[index+nlab*MaskNii->nvox]/normalized_weight;
                }

                MaskNiiPtr[index]= ( (MaskNiiPtr[index] / normalized_weight) > 0.2f )?1:0;
            }

            // set everything below 0.01 to 0 and renormalise
            normalized_weight=0;
            for(int nlab=0; nlab<maximumNumbClass; nlab++)
            {

                if(ResultingProbPtr[index+nlab*MaskNii->nvox]<0.01){
                    ResultingProbPtr[index+nlab*MaskNii->nvox]=0;
                }
                normalized_weight+=ResultingProbPtr[index+nlab*MaskNii->nvox];
            }
            if(normalized_weight<=0 || normalized_weight!=normalized_weight)
            {
                ResultingProbPtr[index]=1;
                for(int nlab=1; nlab<maximumNumbClass; nlab++)
                {
                    ResultingProbPtr[index+nlab*MaskNii->nvox]=0;
                }
            }
            else
            {

                for(int nlab=0; nlab<maximumNumbClass; nlab++)
                {
                    ResultingProbPtr[index+nlab*MaskNii->nvox]=ResultingProbPtr[index+nlab*MaskNii->nvox]/normalized_weight;
                }
            }

        }
        // if its outside the mask, then set all to 0
        else
        {
            MaskNiiPtr[index]=0;
            ResultingProbPtr[index]=1;
            for(int nlab=1; nlab<maximumNumbClass; nlab++)
            {
                ResultingProbPtr[index+nlab*MaskNii->nvox]=0;
            }
        }
    }


    nifti_image_write(ResultingProb);

    ImageSize * CurrSize = new ImageSize [1]();
    CurrSize->numel=(long)(MaskNii->nx*MaskNii->ny*MaskNii->nz);
    CurrSize->xsize=MaskNii->nx;
    CurrSize->ysize=MaskNii->ny;
    CurrSize->zsize=MaskNii->nz;
    CurrSize->usize=(MaskNii->nu>1)?MaskNii->nu:1;
    CurrSize->tsize=(MaskNii->nt>1)?MaskNii->nt:1;


    float * filledMask=(float *)malloc(MaskNii->nvox * MaskNii->nbyper);
    // Dilate, fill, copy back and erode
    Dillate(MaskNiiPtr,4,CurrSize);
    Close_Forground_ConnectComp<float,float>(static_cast<void*>(MaskNiiPtr),static_cast<void*>(filledMask),CurrSize);
    memcpy(MaskNiiPtr,filledMask, MaskNii->nvox*MaskNii->nbyper);
    Erosion(MaskNiiPtr,3,CurrSize);
    free(filledMask);
    delete [] CurrSize;

    //filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_Mask.nii.gz";
    //nifti_set_filenames(MaskNii,filenameStr.c_str(),0,0);
    //nifti_image_write(MaskNii);

    // Segmenting image with priors
    filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_Segmentation.nii.gz";

    // Convert data from time (nt) to multimodal (nu), as multimodal images should be stored as a 5D
    if(this->target->getPtr()->nt>1)
    {
        this->target->getPtr()->dim[5]=this->target->getPtr()->nu=this->target->getPtr()->nt;
        this->target->getPtr()->nt=1;
    }

    seg_EM SEG(ResultingProb->nt,1,max(this->target->getPtr()->nt,1));
    SEG.SetInputImage(this->target->getPtr());
    SEG.SetMaskImage(MaskNii);
    SEG.SetPriorImage(ResultingProb);
    if(this->getVerbose()>1)
    {
        SEG.SetVerbose(1);
    }
    else
    {
        SEG.SetVerbose(0);
    }
    SEG.SetFilenameOut((char *)filenameStr.c_str());
    SEG.SetMaximalIterationNumber(30);
    SEG.SetMinIterationNumber(this->segMinNumIter);
    SEG.SetBiasField(4,0.1f);
    if(this->segRFalpha>0)
        SEG.SetRelaxation(this->segRFalpha,this->segRFsmo);
    SEG.SetMRF(this->segMRFbeta);
    SEG.Run_EM();

    smartPointer<niftiObject> SegResultImg = new niftiObject(SEG.GetResult());
    nifti_image * SegResult=SegResultImg->getPtr();
    if(this->getVerbose()>0)
        cout << "    - Saving Segmentation: "<<filenameStr<< endl;
    SegResult->cal_min=0;
    SegResult->cal_max=1;
    nifti_image_write(SegResult);

    filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_BiasCorrected.nii.gz";
    smartPointer<niftiObject> BiasFieldCorrected = new niftiObject(SEG.GetBiasCorrected((char *)filenameStr.c_str()));
    if(this->getVerbose()>0)
        cout << "    - Saving Bias Corrected Input Image: "<<filenameStr<< endl;
    nifti_image_write(BiasFieldCorrected->getPtr());


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultingLabelsImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    nifti_image *ResultingLabels = ResultingLabelsImg->getPtr();
    ResultingLabels->dim[0]=ResultingLabels->ndim=3;
    ResultingLabels->dim[4]=ResultingLabels->nt=1;
    ResultingLabels->dim[5]=ResultingLabels->nu=1;
    ResultingLabels->pixdim[5]=ResultingLabels->du=1;
    nifti_update_dims_from_array(ResultingLabels);
    ResultingLabels->datatype = NIFTI_TYPE_FLOAT32;
    ResultingLabels->nbyper = sizeof(float);
    ResultingLabels->data = (void *)malloc(ResultingLabels->nvox * ResultingLabels->nbyper);
    if(ResultingLabels->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    float * ResultingLabelsPtr=static_cast<float *>(ResultingLabels->data);
    float * SegResultPtr=static_cast<float *>(SegResult->data);

   // If using Update
    float * lastLabPtr=NULL;
    float * lastGeoPtr=NULL;
    float * currGeoPtr=NULL;
    smartPointer<niftiDynamicObject> lastLabelImg;
    smartPointer<niftiDynamicObject> lastGeoImg;
    if(this->updateLab==1){
        if(this->geoDB[databasenumb].cPTR()!=NULL){ // If the geoDB database exists, define the names and try to read the images
            string curfilename=this->target->getFilename();
            if(curfilename.find_last_of(SEP)>0)
            {
                curfilename=curfilename.substr(curfilename.find_last_of(SEP)+1,curfilename.size()-curfilename.find_last_of(SEP));
            }
            string lastLabelName=this->infoDB[databasenumb]->getPath()+SEP+curfilename+".nii.gz";
            string lastGeoName=this->geoDB[databasenumb]->getPath()+SEP+curfilename+".nii.gz";

            FILE *lastLabFile;
            FILE *lastGeoFile;
            if((bool)(lastLabFile=fopen(lastLabelName.c_str(), "r")) && (bool)(lastGeoFile=fopen(lastGeoName.c_str(), "r"))){
                fclose(lastLabFile); fclose(lastGeoFile);
                cout<<"    - Updating Label assuming:"<<"\n        Last Label Name   - "<<lastLabelName<<"\n        Last GeoDist Name - "<<lastGeoName<<endl;
                lastLabelImg=smartPointer<niftiDynamicObject>(new niftiDynamicObject(lastLabelName,true));
                lastGeoImg=smartPointer<niftiDynamicObject>(new niftiDynamicObject(lastGeoName,true));

                // if the files exist, then only update the Labels where the current geo is smaller than the previous geo
                lastLabPtr=static_cast<float *>(lastLabelImg->getPtr()->data);
                lastGeoPtr=static_cast<float *>(lastGeoImg->getPtr()->data);
                currGeoPtr=static_cast<float *>(this->resultsGeoDB[databasenumb]->getPtr()->data);
            }
            else{
                cout<<"    - Could not find lastGeoName: "<<lastGeoName<<" or lastLabelName: "<<lastLabelName<<endl;
            }
        }
    }


    for(int index=0; index<MaskNii->nvox; index++)
    {
        // if it is inside the mask
        if(this->targetMask[index])
        {
            int maxIndexSeg=0;
            float maxValSeg=0;
            // Find the tissue with the highest segmentation
            for(int nlab=0; nlab<maximumNumbClass; nlab++)
            {
                if(maxValSeg<SegResultPtr[index+nlab*MaskNii->nvox])
                {
                    maxIndexSeg=nlab;
                    maxValSeg=SegResultPtr[index+nlab*MaskNii->nvox];
                }
            }
            // Find highest probability label which is mapped tp the tissue class maxIndexSeg
            int maxIndexLab=0;
            float maxValLab=0;
            for(int nlab=0; nlab<std::numeric_limits<unsigned char>::max(); nlab++)
            {
                for(int nclas=0; nclas<mapping[nlab].size(); nclas++)
                {
                    //I need to figure something out here
                    if(mapping[nlab][nclas]==maxIndexSeg && (this->resultsInfoDB[databasenumb]->getValue(index,nlab))>maxValLab)
                    {
                        maxIndexLab=nlab;
                        maxValLab=this->resultsInfoDB[databasenumb]->getValue(index,nlab);
                    }
                }
            }

            // If the above fails (should only happen if the segRF in on), then just fall back to label fusion without segmentation
            if(maxValLab==0){
                for(int nlab=0; nlab<std::numeric_limits<unsigned char>::max(); nlab++)
                {
                    for(int nclas=0; nclas<mapping[nlab].size(); nclas++)
                    {
                        //I need to figure something out here
                        if( (this->resultsInfoDB[databasenumb]->getValue(index,nlab))>maxValLab)
                        {
                            maxIndexLab=nlab;
                            maxValLab=this->resultsInfoDB[databasenumb]->getValue(index,nlab);
                        }
                    }
                }
            }

            if( this->updateLab==0 ){
                ResultingLabelsPtr[index]=maxIndexLab;
            }
            else{
                if(lastLabPtr!=NULL && lastGeoPtr!=NULL && currGeoPtr!=NULL){
                    ResultingLabelsPtr[index]=currGeoPtr[index]<lastGeoPtr[index]?maxIndexLab:lastLabPtr[index];
                    currGeoPtr[index]=currGeoPtr[index]<lastGeoPtr[index]?currGeoPtr[index]:lastGeoPtr[index];
                }
                else{
                    ResultingLabelsPtr[index]=maxIndexLab;
                }
            }
        }
        else
        {
            ResultingLabelsPtr[index]=this->resultsInfoDB[databasenumb]->getKeyWithMaxVal(index);
        }

    }


    // Postprocess the data given the Neuromorphometrics protocol
    //this->postprocessNeuromorphometricsFix(ResultingLabelsPtr,ResultingLabels);
    filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_Parcellation.nii.gz";
    nifti_set_filenames(ResultingLabels,filenameStr.c_str(),0,0);
    if(this->getVerbose()>0)
        cout << "    - Saving Labels: "<<filenameStr<< endl;
    float min=FLT_MAX;
    float max=-FLT_MAX;
    for(long i=0; i<(long)(ResultingLabels->nvox); i++)
    {
        max=ResultingLabelsPtr[i]>max?ResultingLabelsPtr[i]:max;
        min=ResultingLabelsPtr[i]<min?ResultingLabelsPtr[i]:min;
    }
    ResultingLabels->cal_min=min;
    ResultingLabels->cal_max=max;
    nifti_image_write(ResultingLabels);


    // Save brain mask (all vox with label>=2)
    if(this->saveGeo)
    {
        // Save geodesic distance
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving Geo: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }

    string filenameXML=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".xml";
    mapImage * currentInfoDB=this->resultsInfoDB[databasenumb].cPTR();
    this->postprocessXML(filenameXML, xml_file_name, mapping, BiasFieldCorrected->getPtr(),SegResult,ResultingLabels, currentInfoDB);

}

void GIF::postprocessNeuromorphometricsFix(float * ResultingLabelsPtr, nifti_image * ResultingLabels)
{

  smartPointer<niftiObject> BufferLabelsImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
  nifti_image *BufferLabels = BufferLabelsImg->getPtr();
  BufferLabels->dim[0]=BufferLabels->ndim=3;
  BufferLabels->dim[4]=BufferLabels->nt=1;
  BufferLabels->dim[5]=BufferLabels->nu=1;
  BufferLabels->pixdim[5]=BufferLabels->du=1;
  nifti_update_dims_from_array(BufferLabels);
  BufferLabels->datatype = NIFTI_TYPE_FLOAT32;
  BufferLabels->nbyper = sizeof(float);
  BufferLabels->data = (void *)malloc(ResultingLabels->nvox * ResultingLabels->nbyper);
  if(BufferLabels->data==NULL)
  {
      cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
      exit(1);
  }
  float * BufferLabelsPtr=static_cast<float *>(BufferLabels->data);
  memcpy(BufferLabelsPtr, ResultingLabelsPtr, BufferLabels->nvox*BufferLabels->nbyper);


  for(long indexZ=1; indexZ<(ResultingLabels->nz-1); indexZ++){
      for(long indexY=1; indexY<(ResultingLabels->ny-1); indexY++){
          for(long indexX=1; indexX<(ResultingLabels->nx-1); indexX++){
              int indexcur=indexX+indexY*ResultingLabels->nx+indexZ*ResultingLabels->ny*ResultingLabels->nx;
              float curval=BufferLabelsPtr[indexcur];
              if( this->targetMask[indexcur] &&
                      curval!= 52 &&
                      curval!= 53 &&
                      curval!= 47 &&
                      curval!= 50 &&
                      curval!= 51 &&
                      curval!= 46 &&
                      curval!= 45)
              {
                  int shiftrealsize=1;
                  int shiftspacing=1;

                  int stop=0;
                  for(int shiftz=-shiftrealsize; shiftz<=shiftrealsize; shiftz+=shiftspacing){
                      for(int shifty=-shiftrealsize; shifty<=shiftrealsize; shifty+=shiftspacing){
                          for(int shiftx=-shiftrealsize; shiftx<=shiftrealsize; shiftx+=shiftspacing){
                              int index1=(indexX+shiftx)+
                                      ResultingLabels->nx*(indexY+shifty)+
                                      ResultingLabels->ny*ResultingLabels->nx*(indexZ+shiftz);
                              int index2=(indexX-shiftx)+
                                      ResultingLabels->nx*(indexY-shifty)+
                                      ResultingLabels->ny*ResultingLabels->nx*(indexZ-shiftz);
                              float curval1=BufferLabelsPtr[index1];
                              float curval2=BufferLabelsPtr[index2];
                              if(stop==0 && (fabs(shiftx)+fabs(shifty)+fabs(shiftz))<2 ){
                                  if(curval1==46){
                                      if(curval2==47|| curval2==51|| curval2==53){
                                          ResultingLabelsPtr[indexcur]=46;
                                          stop=1;
                                      }
                                      else{
                                          ResultingLabelsPtr[indexcur]=BufferLabelsPtr[indexcur];

                                      }
                                  }
                                  else if(curval1==45 ){
                                      if(curval2==52 || curval2==50 || curval2==47 ){
                                          ResultingLabelsPtr[indexcur]=45;
                                          stop=1;
                                      }
                                      else{
                                          ResultingLabelsPtr[indexcur]=BufferLabelsPtr[indexcur];

                                      }
                                  }
                                  else  if(curval2==47|| curval2==51|| curval2==53){
                                      if(curval2==46 ){
                                          ResultingLabelsPtr[indexcur]=46;
                                          stop=1;
                                      }
                                      else{
                                          ResultingLabelsPtr[indexcur]=BufferLabelsPtr[indexcur];

                                      }
                                  }
                                  else if(curval2==52 || curval2==50 || curval2==47 ){
                                      if(curval2==45 ){
                                          ResultingLabelsPtr[indexcur]=45;
                                          stop=1;
                                      }
                                      else{
                                          ResultingLabelsPtr[indexcur]=BufferLabelsPtr[indexcur];
                                      }
                                  }
                                  else{
                                      ResultingLabelsPtr[indexcur]=BufferLabelsPtr[indexcur];

                                  }
                              }

                          }
                      }
                  }

              }
              else{
                  ResultingLabelsPtr[indexX+indexY*ResultingLabels->nx+indexZ*ResultingLabels->ny*ResultingLabels->nx]=BufferLabelsPtr[indexX+indexY*ResultingLabels->nx+indexZ*ResultingLabels->ny*ResultingLabels->nx];

              }
          }
      }
  }
}

bool GIF::postprocessXML(string saveFilename, string baseXMLFilename, vector< vector<int> > mapping, nifti_image * BiasFieldCorrected, nifti_image * TissueSegmentation, nifti_image * categoricalSegmentation, mapImage * currentInfoDB)
{

  // ****************************** STARTING XML SECTION ****************************

  // Parsing input XML
  rapidxml::xml_document<> * baseXMLDocument=new rapidxml::xml_document<>;
  baseXMLDocument->parse<0>(xmlReadFile(baseXMLFilename));
  // Find <document> node
  rapidxml::xml_node<> *document_node=NULL;
  for (rapidxml::xml_node<> *base_node=baseXMLDocument->first_node(); base_node; base_node=base_node->next_sibling())
  {
    if(strcmp(base_node->name(),"document")==0)
    {
      document_node=base_node;
    }
  }
  if(document_node==NULL){fprintf(stderr,"* Error: <document> node not found in XML file\n"); return 1;}

  // Find <labels> <tissues> node
  rapidxml::xml_node<> *labels_node=NULL;
  rapidxml::xml_node<> *tissues_node=NULL;
  for (rapidxml::xml_node<> *base_node=document_node->first_node(); base_node; base_node=base_node->next_sibling())
  {
    if(strcmp(base_node->name(),"labels")==0)
    {
      labels_node=base_node;
    }
    if(strcmp(base_node->name(),"tissues")==0)
    {
      tissues_node=base_node;
    }
  }
  if(labels_node==NULL){fprintf(stderr,"* Error: <labels> node not found inside the <document> node\n"); return 1;}
  if(tissues_node==NULL){fprintf(stderr,"* Error: <tissues> node not found inside the <document> node\n"); return 1;}


  // Find and fill up the other nodes
  for (rapidxml::xml_node<> *base_node=document_node->first_node(); base_node; base_node=base_node->next_sibling())
  {
    if(strcmp(base_node->name(),"labels")==0)
    {
      for (rapidxml::xml_node<> *current_node=base_node->first_node(); current_node; current_node=current_node->next_sibling())
      {
        if(strcmp(current_node->name(),"item")==0)
        {
          bool exists=false;
          string instr=xmlGetTextFromXmlNode(current_node,(char*)"number",&exists);
          if (exists==0){
            fprintf(stderr, "ERROR: can't find <number> node in <label> item \n");
            exit(0);
          }
          const char *inptr=instr.c_str();
          char * endptr=NULL;
          int label = (int)(strtol(inptr, &endptr, 10));
          if (endptr == inptr){
            fprintf(stderr, "ERROR: can't convert <number>%s</number>node string to number in <label> item \n",inptr);
            exit(0);
          }
          double volumeProb=this->postprocessEstimateLabelProbabilisticVolume(label,&mapping, TissueSegmentation, currentInfoDB);
          xmlAddNodeToItem(baseXMLDocument,current_node,"volumeProb",std::to_string(volumeProb));
          double volumeCat=this->postprocessEstimateLabelCategoricalVolume(label,categoricalSegmentation);
          xmlAddNodeToItem(baseXMLDocument,current_node,"volumeCat",std::to_string(volumeCat));
          if(this->getVerbose()>0)
          std::cout<<"Vol -> \tLabel="<<label<<"  VolumeProb="<<volumeProb<<"  VolumeCat="<<volumeCat<<std::endl;
          xmlAddNodeToItem(baseXMLDocument,current_node,"units","mm3");
        }
      }
    }
    if(strcmp(base_node->name(),"tissues")==0)
    {
      for (rapidxml::xml_node<> *current_node=base_node->first_node(); current_node; current_node=current_node->next_sibling())
      {
        if(strcmp(current_node->name(),"item")==0)
        {
          bool exists=false;
          char * endptr;
          const char *inptr=xmlGetTextFromXmlNode(current_node,(char*)"number",&exists).c_str();
          int tissue = (int)(strtol(inptr, &endptr, 10));
          if (endptr == inptr){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <tissues> item -> %s\n",inptr);
              exit(0);
          }
          double volume=this->postprocessEstimateTissueProbabilisticVolume(tissue, TissueSegmentation);


          double volumeProb=this->postprocessEstimateTissueProbabilisticVolume(tissue, TissueSegmentation);
          xmlAddNodeToItem(baseXMLDocument,current_node,"volumeProb",std::to_string(volumeProb));
          double volumeCat=this->postprocessEstimateTissueCategoricalVolume(tissue,TissueSegmentation);
          xmlAddNodeToItem(baseXMLDocument,current_node,"volumeCat",std::to_string(volumeCat));
  	  if(this->getVerbose()>0)
            std::cout<<"Vol -> \tTissue="<<tissue<<"  VolumeProb="<<volumeProb<<"  VolumeCat="<<volumeCat<<std::endl;
          xmlAddNodeToItem(baseXMLDocument,current_node,"units","mm3");
        }
      }
    }
    if(strcmp(base_node->name(),"snr")==0)
    {
      for (rapidxml::xml_node<> *current_node=base_node->first_node(); current_node; current_node=current_node->next_sibling())
      {
        if(strcmp(current_node->name(),"item")==0)
        {
          bool exists1=false;
          bool exists2=false;
          double snr=0;
          string instr1=xmlGetTextFromXmlNode(current_node,(char*)"tissue",&exists1);
          string instr2=xmlGetTextFromXmlNode(current_node,(char*)"label",&exists2);
          const char *inptr1=instr1.c_str();
          const char *inptr2=instr2.c_str();
          char * endptr1=NULL;
          char * endptr2=NULL;

          if(exists1==1 && exists2==1){
            fprintf(stderr, "ERROR: the <snr> node item can only have either a <label> or a <tissue> item and not both\n");
            exit(0);
          }
          if(exists1==0 && exists2==0){
            fprintf(stderr, "ERROR: the <snr> node item has to have either a <label> or a <tissue> item\n");
            exit(0);
          }
          if(exists1){
            int tissue = (int)(strtol(inptr1, &endptr1, 10));
            if (endptr1 == inptr1){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <tissue> item within <snr> -> %s\n",inptr1);
              exit(0);
            }
            string tissuename=xmlGetNameFromItemWithNumber(tissues_node,tissue,&exists1);
            if(exists1==0){
              fprintf(stderr, "ERROR: <tissue>%d</tissue> requested from <snr> node does not exit \n",tissue);
              exit(0);
            }
            snr=this->postprocessEstimateTissueSNR(tissue, BiasFieldCorrected, TissueSegmentation);
	    if(this->getVerbose()>0)
               std::cout<<"SNR -> \tTissue="<<tissue<<"  SNR="<<snr<<std::endl;
          }
          if(exists2){
            int label = (int)(strtol(inptr2, &endptr2, 10));
            if (endptr2 == inptr2){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <label> item within <snr> -> %s\n",inptr2);
              exit(0);
            }
            string labelname=xmlGetNameFromItemWithNumber(labels_node,label,&exists2);
            if(exists2==0){
              fprintf(stderr, "ERROR: <label>%d</label> requested from <snr> node does not exit \n",label);
              exit(0);
            }
            snr=this->postprocessEstimateLabelSNR(label,mapping, BiasFieldCorrected, TissueSegmentation, currentInfoDB);
	    if(this->getVerbose()>0)
              std::cout<<"SNR -> \tLabel="<<label<<"  SNR="<<snr<<std::endl;
          }
	std::cout << std::to_string(snr)<<endl;
        xmlAddNodeToItem(baseXMLDocument,current_node,"value",std::to_string(snr));
        }
      }
    }
    if(strcmp(base_node->name(),"cnr")==0)
    {
      for (rapidxml::xml_node<> *current_node=base_node->first_node(); current_node; current_node=current_node->next_sibling())
      {
        if(strcmp(current_node->name(),"item")==0)
        {

          bool exists_tissue_1=false;
          bool exists_tissue_2=false;
          bool exists_label_1=false;
          bool exists_label_2=false;
          double cnr=0;
          string instr_tissue_1=xmlGetTextFromXmlNode(current_node,(char*)"tissue1",&exists_tissue_1);
          string instr_tissue_2=xmlGetTextFromXmlNode(current_node,(char*)"tissue2",&exists_tissue_2);
          string instr_label_1=xmlGetTextFromXmlNode(current_node,(char*)"label1",&exists_label_1);
          string instr_label_2=xmlGetTextFromXmlNode(current_node,(char*)"label2",&exists_label_2);
          const char *inptr_tissue_1=instr_tissue_1.c_str();
          const char *inptr_tissue_2=instr_tissue_2.c_str();
          const char *inptr_label_1=instr_label_1.c_str();
          const char *inptr_label_2=instr_label_2.c_str();
          char * endptr1=NULL;
          char * endptr2=NULL;

          bool exists_tissue=(exists_tissue_1&&exists_tissue_2);
          bool exists_label=(exists_label_1&&exists_label_2);

          if(exists_tissue==1 && exists_label==1){
            fprintf(stderr, "ERROR: the <cnr> node item can only have either two <label> or to <tissue> items and not a mix\n");
            exit(0);
          }
          if(exists_tissue==0 && exists_label==0){
            fprintf(stderr, "ERROR: the <cnr> node item has to have either two <label> or to <tissue> items\n");
            exit(0);
          }
          if(exists_tissue){
            int tissue1 = (int)(strtol(inptr_tissue_1, &endptr1, 10));
            int tissue2 = (int)(strtol(inptr_tissue_2, &endptr2, 10));
            if (endptr1 == inptr_tissue_1){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <tissue1> item within <cnr> -> %s\n",inptr_tissue_1);
              exit(0);
            }
            if (endptr2 == inptr_tissue_2){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <tissue2> item within <cnr> -> %s\n",inptr_tissue_2);
              exit(0);
            }
            string tissuename1=xmlGetNameFromItemWithNumber(tissues_node,tissue1,&exists_tissue_1);
            if(exists_tissue_1==0){
              fprintf(stderr, "ERROR: <tissue1>%d</tissue1> requested from <cnr> node does not exit \n",tissue1);
              exit(0);
            }
            string tissuename2=xmlGetNameFromItemWithNumber(tissues_node,tissue2,&exists_tissue_2);
            if(exists_tissue_2==0){
              fprintf(stderr, "ERROR: <tissue2>%d</tissue2> requested from <cnr> node does not exit \n",tissue2);
              exit(0);
            }
            cnr=this->postprocessEstimateTissueCNR(tissue1,tissue2, BiasFieldCorrected, TissueSegmentation);
          if(this->getVerbose()>0)
            std::cout<<"CNR -> \tTissues="<<tissue1<<","<<tissue2<<"CNR="<<cnr<<std::endl;
          }
          if(exists_label){
            int label1 = (int)(strtol(inptr_label_1, &endptr1, 10));
            int label2 = (int)(strtol(inptr_label_2, &endptr2, 10));
            if (endptr1 == inptr_label_1){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <label1> item within <cnr> -> %s\n",inptr_label_1);
              exit(0);
            }
            if (endptr2 == inptr_label_2){
              fprintf(stderr, "ERROR: can't convert <number> node string to number in <label2> item within <cnr> -> %s\n",inptr_label_2);
              exit(0);
            }
            string labelname1=xmlGetNameFromItemWithNumber(labels_node,label1,&exists_label_1);
            if(exists_label_1==0){
              fprintf(stderr, "ERROR: <label1>%d</label1> requested from <cnr> node does not exit \n",label1);
              exit(0);
            }
            string labelname2=xmlGetNameFromItemWithNumber(labels_node,label2,&exists_label_2);
            if(exists_label_2==0){
              fprintf(stderr, "ERROR: <label2>%d</label2> requested from <cnr> node does not exit \n",label2);
              exit(0);
            }

            cnr=this->postprocessEstimateLabelCNR(label1,label2,mapping, BiasFieldCorrected, TissueSegmentation, currentInfoDB);
          if(this->getVerbose()>0)
            std::cout<<"CNR -> \tLabels="<<label1<<","<<label2<<"CNR="<<cnr<<std::endl;
          }
          xmlAddNodeToItem(baseXMLDocument,current_node,"value",std::to_string(cnr));
        }
      }
    }
  }

  // Save XML document to file stream
  std::ofstream streamXMLfile;
  streamXMLfile.open(saveFilename.c_str());
  streamXMLfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
  streamXMLfile << baseXMLDocument[0];
  streamXMLfile.close();

  // ****************************** FINISHED XML SECTION ****************************

}

double GIF::postprocessEstimateLabelCNR(int label1,int label2,
                                   vector< vector<int> > mapping,
                                   nifti_image * BiasFieldCorrected,
                                   nifti_image * TissueSegmentation,
                                   mapImage * currentInfoDB)
{
  double cnr=0;
  std::cout<<"WARNING: CNR estimation between tissues is not implemented"<<std::endl;
  return cnr;
}

double GIF::postprocessEstimateTissueCNR(int tissue1, int tissue2,
                                    nifti_image * BiasFieldCorrected,
                                    nifti_image * TissueSegmentation)
{

  if(tissue1>TissueSegmentation->nt)
  {
    cerr << "Warning: <tissue1>" << tissue1<<"</tissue1> does not exist when estimating the CNR"<< endl;
    return 0;
  }
  if(tissue2>TissueSegmentation->nt)
  {
    cerr << "Warning: <tissue2>" << tissue2<<"</tissue2> does not exist when estimating the CNR"<< endl;
    return 0;
  }

  float * TissueSegmentationPtr=static_cast<float *>(TissueSegmentation->data);
  float * BiasFieldCorrectedPtr=static_cast<float *>(BiasFieldCorrected->data);

  double mean1=0;
  double mean2=0;
  double variance1=0;
  double variance2=0;
  double sum1weight=0;
  double sum2weight=0;
  int nvox=(BiasFieldCorrected->nx*BiasFieldCorrected->ny*BiasFieldCorrected->nz);
  for(int index=0; index<nvox; index++)
  {
      // if it is inside the mask
      if(this->targetMask[index])
      {
          // Find highest probability label which is mapped tp the tissue class maxIndexSeg
          mean1+=(TissueSegmentationPtr[index+tissue1*nvox]>0.95)*BiasFieldCorrectedPtr[index];
          sum1weight+=(TissueSegmentationPtr[index+tissue1*nvox]>0.95);
          mean2+=(TissueSegmentationPtr[index+tissue2*nvox]>0.95)*BiasFieldCorrectedPtr[index];
          sum2weight+=(TissueSegmentationPtr[index+tissue2*nvox]>0.95);
      }
  }
  mean1=mean1/sum1weight;
  mean2=mean2/sum2weight;

  for(int index=0; index<nvox; index++)
  {
      // if it is inside the mask
      if(this->targetMask[index])
      {
          // Find highest probability label which is mapped tp the tissue class maxIndexSeg
          variance1+=(TissueSegmentationPtr[index+tissue1*nvox]>0.95)*(BiasFieldCorrectedPtr[index]-mean1)*(BiasFieldCorrectedPtr[index]-mean1);
          variance2+=(TissueSegmentationPtr[index+tissue2*nvox]>0.95)*(BiasFieldCorrectedPtr[index]-mean2)*(BiasFieldCorrectedPtr[index]-mean2);
      }
  }
  variance1=(variance1/sum1weight);
  variance2=(variance2/sum2weight);


  return abs(mean1-mean2)/sqrt((variance1+variance2)/2.0f);
}

double GIF::postprocessEstimateLabelSNR(int label,
                                        vector< vector<int> > mapping,
                                   nifti_image * BiasFieldCorrected,
                                   nifti_image * TissueSegmentation,
                                   mapImage * currentInfoDB)
{
  std::cout<<"WARNING: SNR estimation between tissues is not implemented"<<std::endl;
  return 0;
}

double GIF::postprocessEstimateTissueSNR(int tissue,
                                    nifti_image * BiasFieldCorrected,
                                    nifti_image * TissueSegmentation)
{

  if(tissue>TissueSegmentation->nt)
  {
    cerr << "Warning: <tissue>" << tissue<<"</tissue> does not exist when estimating the SNR"<< endl;
    return 0;
  }

  float * TissueSegmentationPtr=static_cast<float *>(TissueSegmentation->data);
  float * BiasFieldCorrectedPtr=static_cast<float *>(BiasFieldCorrected->data);

  double mean1=0;
  double variance1=0;
  double sum1weight=0;
  int nvox=(BiasFieldCorrected->nx*BiasFieldCorrected->ny*BiasFieldCorrected->nz);
  for(int index=0; index<nvox; index++)
  {
    // if it is inside the mask
    if(this->targetMask[index])
    {
      // Find highest probability label which is mapped tp the tissue class maxIndexSeg
      mean1+=(TissueSegmentationPtr[index+tissue*nvox]>0.95)*BiasFieldCorrectedPtr[index];
      sum1weight+=(TissueSegmentationPtr[index+tissue*nvox]>0.95);
    }
  }
  mean1=mean1/sum1weight;

  for(int index=0; index<nvox; index++)
  {
    // if it is inside the mask
    if(this->targetMask[index])
    {
      // Find highest probability label which is mapped tp the tissue class maxIndexSeg
      variance1+=(TissueSegmentationPtr[index+tissue*nvox]>0.95)*(BiasFieldCorrectedPtr[index]-mean1)*(BiasFieldCorrectedPtr[index]-mean1);
    }
  }
  variance1=(variance1/sum1weight);


  return abs(mean1)/sqrt(variance1);

}

double GIF::postprocessEstimateLabelProbabilisticVolume(int label,
                                      vector< vector<int> > *mapping,
                                      nifti_image * TissueSegmentation,
                                      mapImage * infoDB)
{


  double volume=0;
  float * TissueSegmentationPtr=static_cast<float *>(TissueSegmentation->data);


  int nvox=(TissueSegmentation->nx*TissueSegmentation->ny*TissueSegmentation->nz);

  for(size_t index=0; index<nvox; index++)
  {
      if(this->targetMask[index])
      {
        // Get the label prior probability
        float curprob=infoDB->getValue(index,label);

        // if the prior is above 0, then get the normalising pribability for each class
        if(curprob>0){
          float sumprob=0;
          for(int labelClass=0; labelClass<(*mapping)[label].size(); labelClass++){
            for(int testLab=0; testLab<255; testLab++){
              for(int testLabClass=0; testLabClass<(*mapping)[testLab].size(); testLabClass++)
              {
                if((*mapping)[testLab][testLabClass]==(*mapping)[label][labelClass]){
                  sumprob+=infoDB->getValue(index,testLab);
                }
              }
            }
            if(sumprob==0 || curprob>sumprob){
              cerr << "WARNING: WTF?!?... how is the sumprob 0 if curprob is above 0"<< endl;
            }
            else{
              // The volume is the posterior tissue probability multiplied by the prior ratio
              volume+=TissueSegmentationPtr[index+(*mapping)[label][labelClass] * nvox]*(curprob/sumprob);
            }
          }
        }
      }
      else{
        if(label==0){
          volume+=1;
        }
      }
  }

  return volume*(double)(TissueSegmentation->dx*TissueSegmentation->dy*TissueSegmentation->dz);
}

double GIF::postprocessEstimateLabelCategoricalVolume(int label,
                                      nifti_image * categoricalSegmentation)
{


  double volume=0;
  float * categoricalSegmentationPtr=static_cast<float *>(categoricalSegmentation->data);


  int nvox=(categoricalSegmentation->nx*categoricalSegmentation->ny*categoricalSegmentation->nz);

  for(size_t index=0; index<nvox; index++)
  {
      if(this->targetMask[index])
      {
        // Get the label prior probability
        volume+=(categoricalSegmentationPtr[index]==(float)(label));
      }
  }
  return volume*(double)(categoricalSegmentation->dx*categoricalSegmentation->dy*categoricalSegmentation->dz);
}



double GIF::postprocessEstimateTissueProbabilisticVolume(int tissue,
                                       nifti_image * TissueSegmentation)
{
  float * TissueSegmentationPtr=static_cast<float *>(TissueSegmentation->data);

  if(tissue>=TissueSegmentation->nt)
  {
    cerr << "Warning: <tissue>" << tissue<<"</tissue> does not exist when estimating the tissue volume from segmentation"<< endl;
    return 0;
  }
  double volume=0;
  int nvox=(TissueSegmentation->nx*TissueSegmentation->ny*TissueSegmentation->nz);
  for(int index=0; index<nvox; index++)
  {
    // if it is inside the mask
    if(this->targetMask[index])
    {
      // add the probability for each <index> of a <tissue>
      if(TissueSegmentationPtr[index+tissue*nvox]>0.01)
      { // This check is to avoid adding volume for voxels with trace probability
        volume+=(double)(TissueSegmentationPtr[index+tissue*nvox]);
      }
    }
  }
  return volume*(double)(TissueSegmentation->dx*TissueSegmentation->dy*TissueSegmentation->dz);
}

double GIF::postprocessEstimateTissueCategoricalVolume(int tissue,
                                       nifti_image * TissueSegmentation)
{
  float * TissueSegmentationPtr=static_cast<float *>(TissueSegmentation->data);

  if(tissue>=TissueSegmentation->nt)
  {
    cerr << "Warning: <tissue>" << tissue<<"</tissue> does not exist when estimating the tissue volume from segmentation"<< endl;
    return 0;
  }
  double volume=0;
  int nvox=(TissueSegmentation->nx*TissueSegmentation->ny*TissueSegmentation->nz);
  for(int index=0; index<nvox; index++)
  {
    // if it is inside the mask
    if(this->targetMask[index])
    {
      // add the probability for each <index> of a <tissue>
      bool test=true;
      for(int tissueIndex=0; tissueIndex<TissueSegmentation->nt; tissueIndex++){
        if(TissueSegmentationPtr[index+tissue*nvox]<TissueSegmentationPtr[index+tissueIndex*nvox])
        { // if prob for tissue is smaller than any prob for tissueIndex, then set to false and break loop
          test=false;
          // this stops the for loop
          tissueIndex=TissueSegmentation->nt;
        }
        volume+=(double)(test);
      }
    }
  }
  return volume*(double)(TissueSegmentation->dx*TissueSegmentation->dy*TissueSegmentation->dz);
}


void GIF::processAndSaveFlowAsProb(unsigned int databasenumb)
{
    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }
    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultingProbImg = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingProb =  ResultingProbImg->getPtr();
    ResultingProb->dim[0]=ResultingProb->ndim=4;
    ResultingProb->dim[4]=ResultingProb->nt=this->infoDB[databasenumb]->getDatasetNoDataPtr(0)->nt;
    this->infoDB[databasenumb]->unloadDataFromDataset(0);
    ResultingProb->dim[5]=ResultingProb->nu=1;
    ResultingProb->pixdim[5]=ResultingProb->du=1;
    nifti_update_dims_from_array(ResultingProb);
    ResultingProb->datatype = NIFTI_TYPE_FLOAT32;
    ResultingProb->nbyper = sizeof(float);
    ResultingProb->data = (void *)malloc(ResultingProb->nvox * ResultingProb->nbyper);
    if(ResultingProb->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";

    nifti_set_filenames(ResultingProb,filenameStr.c_str(),0,0);

    float * ResultingProbPtr=static_cast<float *>(ResultingProb->data);

    if(this->getVerbose()>0)
        cout << "    - Saving Prob Information: "<<filenameStr<< endl;

    int vol3d=ResultingProb->nx*ResultingProb->ny*ResultingProb->nz;
    for(int index=0; index<vol3d; index++)
    {
        float normalized_weight=0;
        for(int nlab=0; nlab<ResultingProb->nt; nlab++)
        {
            ResultingProbPtr[index+nlab*vol3d]=this->resultsInfoDB[databasenumb]->getValue(index,nlab);
            normalized_weight+=ResultingProbPtr[index+nlab*vol3d];
        }
        if(normalized_weight==0 || normalized_weight!=normalized_weight)
        {
            ResultingProbPtr[index]=1;
            for(int nlab=1; nlab<ResultingProb->nt; nlab++)
            {
                ResultingProbPtr[index+nlab*vol3d]=0;
            }
        }
        else
        {
            for(int nlab=0; nlab<ResultingProb->nt; nlab++)
            {
                ResultingProbPtr[index+nlab*vol3d]=ResultingProbPtr[index+nlab*vol3d]/normalized_weight;
            }
        }
    }
    nifti_image_write(ResultingProb);

    if(this->saveGeo){
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving Prob Geo Information: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);

        this->resultsGeoDB[databasenumb]->getPtr()->cal_min=0;
        this->resultsGeoDB[databasenumb]->getPtr()->cal_max=0;

        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }
}

void GIF::processAndSaveFlowAsProbSeg(unsigned int databasenumb)
{

    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // ...
    smartPointer<niftiObject> ResultingProbImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    nifti_image *ResultingProb = ResultingProbImg->getPtr();
    ResultingProb->dim[0]=ResultingProb->ndim=4;
    ResultingProb->dim[4]=ResultingProb->nt=this->infoDB[databasenumb]->getDatasetNoDataPtr(0)->nt;
    this->infoDB[databasenumb]->unloadDataFromDataset(0);
    ResultingProb->dim[5]=ResultingProb->nu=1;
    ResultingProb->pixdim[5]=ResultingProb->du=1;
    nifti_update_dims_from_array(ResultingProb);
    ResultingProb->datatype = NIFTI_TYPE_FLOAT32;
    ResultingProb->nbyper = sizeof(float);
    ResultingProb->data = (void *)malloc(ResultingProb->nvox * ResultingProb->nbyper);
    if(ResultingProb->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }


    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_prior.nii.gz";

    nifti_set_filenames(ResultingProb,filenameStr.c_str(),0,0);

    float * ResultingProbPtr=static_cast<float *>(ResultingProb->data);

    if(this->getVerbose()>0)
    {
        cout << "    - Segmenting and Saving Information: "<<filenameStr<< endl;
    }

    int vol3d=ResultingProb->nx*ResultingProb->ny*ResultingProb->nz;

    for(int index=0; index<vol3d; index++)
    {
        if(this->targetMask[index])
        {
            float normalized_weight=0;
            for(int nlab=0; nlab<ResultingProb->nt; nlab++)
            {
                ResultingProbPtr[index+nlab*vol3d]=this->resultsInfoDB[databasenumb]->getValue(index,nlab);
                normalized_weight+=ResultingProbPtr[index+nlab*vol3d];
            }
            if(normalized_weight==0 || normalized_weight!=normalized_weight)
            {
                ResultingProbPtr[index]=1;
                for(int nlab=1; nlab<ResultingProb->nt; nlab++)
                {
                    ResultingProbPtr[index+nlab*vol3d]=0;
                }
            }
            else
            {
                for(int nlab=0; nlab<ResultingProb->nt; nlab++)
                {
                    ResultingProbPtr[index+nlab*vol3d]=ResultingProbPtr[index+nlab*vol3d]/normalized_weight;
                }
            }
        }
        else
        {
            ResultingProbPtr[index]=1;
            for(int nlab=1; nlab<ResultingProb->nt; nlab++)
                ResultingProbPtr[index+nlab*vol3d]=0;
        }
    }
    nifti_image_write(ResultingProb);


    // Allocate nifti for mask
    smartPointer<niftiObject> MaskNiiImg = new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    nifti_image *MaskNii = MaskNiiImg->getPtr();
    MaskNii->dim[0]=MaskNii->ndim=3;
    MaskNii->dim[4]=MaskNii->nt=1;
    MaskNii->dim[5]=MaskNii->nu=1;
    nifti_update_dims_from_array(MaskNii);
    MaskNii->datatype = NIFTI_TYPE_INT32;
    MaskNii->nbyper = sizeof(int);
    MaskNii->data = (void *)malloc(MaskNii->nvox * MaskNii->nbyper);
    if(MaskNii->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }
    memcpy(MaskNii->data, this->targetMask, MaskNii->nvox*MaskNii->nbyper);


    filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_seg.nii.gz";

    seg_EM SEG(ResultingProb->nt,1,1);
    SEG.SetInputImage(this->target->getPtr());
    SEG.SetMaskImage(MaskNii);
    SEG.SetPriorImage(ResultingProb);
    SEG.SetVerbose(0);
    SEG.SetFilenameOut((char *)filenameStr.c_str());
    SEG.SetMaximalIterationNumber(30);
    SEG.SetMinIterationNumber(3);
    SEG.SetBiasField(5,0.05);
    SEG.SetMRF(0.1);
    SEG.Run_EM();

    smartPointer<niftiObject> ResultImg = new niftiObject(SEG.GetResult());
    nifti_image * Result=ResultImg->getPtr();

    float * ResultPtr=static_cast<float *>(Result->data);
    for(int index=0; index<vol3d; index++)
    {
        if(this->targetMask[index]==0)
        {
            ResultPtr[index]=1;
        }
    }
    nifti_image_write(Result);

    if(this->saveGeo){
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving ProbSeb Geo Information: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
        this->resultsGeoDB[databasenumb]->getPtr()->cal_min=0;
        this->resultsGeoDB[databasenumb]->getPtr()->cal_max=0;
        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }
}

void GIF::processAndSaveFlowAsInten(unsigned int databasenumb)
{

    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Inten Information: "<<filenameStr<< endl;


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> Result = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImg = Result->getPtr();
    ResultingImg->dim[0]=ResultingImg->ndim=3;
    ResultingImg->dim[4]=ResultingImg->nt=1;
    ResultingImg->dim[5]=ResultingImg->nu=1;
    ResultingImg->pixdim[5]=ResultingImg->du=1;
    nifti_update_dims_from_array(ResultingImg);
    ResultingImg->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImg->nbyper = sizeof(float);
    ResultingImg->data = (void *)malloc(ResultingImg->nvox * ResultingImg->nbyper);
    if(ResultingImg->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    nifti_set_filenames(ResultingImg,filenameStr.c_str(),0,0);

    float * ResultingImgPtr=static_cast<float *>(ResultingImg->data);



    for(int index=0; index<(ResultingImg->nx*ResultingImg->ny*ResultingImg->nz); index++)
    {
        //if(this->targetMask[index]){
        ResultingImgPtr[index]=this->resultsInfoDB[databasenumb]->getValue(index,1)/this->resultsInfoDB[databasenumb]->getValue(index,0);
        //}
        //else{
        //    ResultingImgPtr[index]=0;
        //}
    }
    ResultingImg->scl_inter=atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    nifti_image_write(ResultingImg);

    if(this->saveGeo){
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving Inten Geo Information: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
        this->resultsGeoDB[databasenumb]->getPtr()->cal_min=0;
        this->resultsGeoDB[databasenumb]->getPtr()->cal_max=2;
        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }

}


void GIF::processAndSaveFlowAsIntenSort(unsigned int databasenumb)
{

    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Inten Information: "<<filenameStr<< endl;


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> Result = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImg = Result->getPtr();
    ResultingImg->dim[0]=ResultingImg->ndim=3;
    ResultingImg->dim[4]=ResultingImg->nt=1;
    ResultingImg->dim[5]=ResultingImg->nu=1;
    ResultingImg->pixdim[5]=ResultingImg->du=1;
    nifti_update_dims_from_array(ResultingImg);
    ResultingImg->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImg->nbyper = sizeof(float);
    ResultingImg->data = (void *)malloc(ResultingImg->nvox * ResultingImg->nbyper);
    if(ResultingImg->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    nifti_set_filenames(ResultingImg,filenameStr.c_str(),0,0);

    float * ResultingImgPtr=static_cast<float *>(ResultingImg->data);



    for(int index=0; index<(ResultingImg->nx*ResultingImg->ny*ResultingImg->nz); index++)
    {
        float tmpIntSum=0;
        float tmpSum=0;
        for(unsigned char sortlab=0; sortlab<this->sortMaxItems; sortlab++)
        {
            float curval=this->resultsInfoDB[databasenumb]->getValueOrNaN(index,sortlab+this->sortMaxItems);
            tmpIntSum+=(curval!=curval)?0:expf(-this->sortBeta * sortlab)*curval;
            tmpSum+=(curval!=curval)?0:expf(-this->sortBeta  * sortlab);
        }
        ResultingImgPtr[index]=tmpIntSum/tmpSum;
    }
    ResultingImg->scl_inter=atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    float min=FLT_MAX;
    float max=-FLT_MAX;
    for(long i=0; i<(long)(ResultingImg->nvox); i++)
    {
        max=ResultingImgPtr[i]>max?ResultingImgPtr[i]:max;
        min=ResultingImgPtr[i]<min?ResultingImgPtr[i]:min;
    }
    ResultingImg->cal_min=min+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImg->cal_max=max+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    nifti_image_write(ResultingImg);


    if(this->saveGeo){
        string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
        if(this->getVerbose()>0)
            cout << "    - Saving Inten Geo Information: "<<filenameGeoStr<< endl;
        this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
        nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());
    }

}


void GIF::processAndSaveFlowAsIntenMAP(unsigned int databasenumb)
{

    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStrMAP     =  this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_MAP.nii.gz";
    string filenameStrExpec   =  this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_Expec.nii.gz";
    string filenameStrNumModes=  this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_NumModes.nii.gz";
    string filenameStrSamples=  this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_Samples.nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Inten Information: "<<filenameStrMAP<< endl;


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultMAP = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImgMAP = ResultMAP->getPtr();
    ResultingImgMAP->dim[0]=ResultingImgMAP->ndim=3;
    ResultingImgMAP->dim[4]=ResultingImgMAP->nt=1;
    ResultingImgMAP->dim[5]=ResultingImgMAP->nu=1;
    ResultingImgMAP->pixdim[5]=ResultingImgMAP->du=1;
    nifti_update_dims_from_array(ResultingImgMAP);
    ResultingImgMAP->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImgMAP->nbyper = sizeof(float);
    nifti_set_filenames(ResultingImgMAP,filenameStrMAP.c_str(),0,0);
    ResultingImgMAP->data = (void *)malloc(ResultingImgMAP->nvox * ResultingImgMAP->nbyper);
    float * ResultingImgMapPtr=static_cast<float *>(ResultingImgMAP->data);
    if(ResultingImgMAP->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultExpec = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImgExpec = ResultExpec->getPtr();
    ResultingImgExpec->dim[0]=ResultingImgExpec->ndim=3;
    ResultingImgExpec->dim[4]=ResultingImgExpec->nt=1;
    ResultingImgExpec->dim[5]=ResultingImgExpec->nu=1;
    ResultingImgExpec->pixdim[5]=ResultingImgExpec->du=1;
    nifti_update_dims_from_array(ResultingImgExpec);
    ResultingImgExpec->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImgExpec->nbyper = sizeof(float);
    nifti_set_filenames(ResultingImgExpec,filenameStrExpec.c_str(),0,0);
    ResultingImgExpec->data = (void *)malloc(ResultingImgExpec->nvox * ResultingImgExpec->nbyper);
    float * ResultingImgExpecPtr=static_cast<float *>(ResultingImgExpec->data);
    if(ResultingImgExpec->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultNumModes = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImgNumModes = ResultNumModes->getPtr();
    ResultingImgNumModes->dim[0]=ResultingImgNumModes->ndim=3;
    ResultingImgNumModes->dim[4]=ResultingImgNumModes->nt=1;
    ResultingImgNumModes->dim[5]=ResultingImgNumModes->nu=1;
    ResultingImgNumModes->pixdim[5]=ResultingImgNumModes->du=1;
    nifti_update_dims_from_array(ResultingImgNumModes);
    ResultingImgNumModes->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImgNumModes->nbyper = sizeof(float);
    nifti_set_filenames(ResultingImgNumModes,filenameStrNumModes.c_str(),0,0);
    ResultingImgNumModes->data = (void *)malloc(ResultingImgNumModes->nvox * ResultingImgNumModes->nbyper);
    float * ResultingImgNumModesPtr=static_cast<float *>(ResultingImgNumModes->data);
    if(ResultingImgNumModes->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> ResultSamples = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImgSamples = ResultSamples->getPtr();
    ResultingImgSamples->dim[0]=ResultingImgSamples->ndim=4;
    ResultingImgSamples->dim[4]=ResultingImgSamples->nt=5;
    ResultingImgSamples->dim[5]=ResultingImgSamples->nu=1;
    ResultingImgSamples->pixdim[5]=ResultingImgSamples->du=1;
    nifti_update_dims_from_array(ResultingImgSamples);
    ResultingImgSamples->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImgSamples->nbyper = sizeof(float);
    nifti_set_filenames(ResultingImgSamples,filenameStrSamples.c_str(),0,0);
    ResultingImgSamples->data = (void *)malloc(ResultingImgSamples->nvox * ResultingImgSamples->nbyper);
    float * ResultingImgSamplesPtr=static_cast<float *>(ResultingImgSamples->data);
    if(ResultingImgSamples->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }




    float maxInten=std::numeric_limits<float>::min();
    float minInten=std::numeric_limits<float>::max();
    float calcMean=0;
    float calcSTD=0;
    float calcCount=0;
    for(int index=0; index<(ResultingImgMAP->nx*ResultingImgMAP->ny*ResultingImgMAP->nz); index++)
    {
        if(this->targetMask[index]){
            for(unsigned char sortlab=0; sortlab<this->sortMaxItems; sortlab++)
            {
                float curIntensity=this->resultsInfoDB[databasenumb]->getValueOrNaN(index,sortlab+this->sortMaxItems);
                if(isnan(curIntensity)==0){

                    if(curIntensity>maxInten){
                        maxInten=curIntensity;
                    }
                    if(curIntensity<minInten){
                        minInten=curIntensity;
                    }

                    calcMean+= (curIntensity);
                    calcSTD += pow(curIntensity,2);
                    calcCount+=1;
                }
            }
        }
    }
    float stepsize=(maxInten-minInten)/500.0f;
    float stdsize=abs(calcSTD/calcCount - pow(calcMean/calcCount,2));


    const long dims[3]={(long)(ResultingImgMAP->dim[1]),
                        (long)(ResultingImgMAP->dim[2]),
                        (long)(ResultingImgMAP->dim[3])};
    if(this->getVerbose()>0)
        cout << "    - Searching for MAP => min:"<<minInten<<"  max:"<<maxInten<<"  std:"<<stdsize<< endl;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(ResultingImgMAP,ResultingImgMapPtr,ResultingImgExpecPtr,ResultingImgSamplesPtr,ResultingImgNumModesPtr,databasenumb,cout,dims,minInten,maxInten,stepsize,stdsize)
#endif
    for(int nz=0; nz<(ResultingImgMAP->nz); nz++)
    {
        unsigned int oldRand=0;
        int minIntenLocal=minInten;
        int maxIntenLocal=maxInten;
        int stepsizeLocal=stepsize;
        int stdsizeLocal=stdsize;
        cout<<nz<<endl;
        for(int ny=0; ny<(ResultingImgMAP->ny); ny++)
        {
            for(int nx=0; nx<(ResultingImgMAP->nx); nx++)
            {
                int index=nx+dims[0]*(ny+(dims[1]*nz));
                float intenVector[30];
                float intenWeight[30];
                float curExpecInt=0;
                float curExpecWeight=0;
                for(unsigned char sortlab=0; sortlab<this->sortMaxItems; sortlab++)
                {

                    float curIntensity=this->resultsInfoDB[databasenumb]->getValueOrNaN(index,sortlab+this->sortMaxItems);
                    float curWeight=this->resultsInfoDB[databasenumb]->getValueOrNaN(index,sortlab);
                    if(isnan(curIntensity) || isnan(curWeight)){
                        intenVector[sortlab]=0;
                        intenWeight[sortlab]=0;
                    }
                    else{
                        curExpecInt+=curIntensity*(-curWeight);
                        curExpecWeight+=(-curWeight);
                        intenVector[sortlab]=curIntensity;
                        intenWeight[sortlab]=(-curWeight);
                    }

                    if(nx==207 && ny==138 && nz==127){
                        cout << "InitWeights:"<< (float)(sortlab) << " - "<<intenVector[sortlab] << " - "<<intenWeight[sortlab] << endl;
                        cout.flush();
                    }
                }
                ResultingImgExpecPtr[index]=curExpecInt/curExpecWeight;

                float maxcurprob=0;
                float finalint=0;

                if(nx==207 && ny==138 && nz==127){
                    cout <<"Sizes:"<< minInten << " - "<<maxInten<< " - "<<stepsize << endl;
                    cout.flush();
                }
                float last3[3]={0,0,0};
                int last3index=0;
                ResultingImgNumModesPtr[index]=0;
                float totalSumProb=0;
                for(float inten=minIntenLocal; inten<maxIntenLocal; inten+=stepsizeLocal){
                    float curprob=0;
                    for(unsigned char sortlab=0; sortlab<this->sortMaxItems; sortlab++)
                    {
                        if(intenWeight[sortlab]>0 && isnan(intenVector[sortlab])==0){
                            curprob+=intenWeight[sortlab]*expf(-pow((intenVector[sortlab]-inten),2)/(0.5*stdsizeLocal/50));
                        }
                    }
                    totalSumProb+=curprob;

                    last3[0]=last3[1];
                    last3[1]=last3[2];
                    last3[2]=curprob;
                    last3index++;
                    if(last3index>3){
                        if(last3[1]>last3[2] && last3[1]>last3[0]){
                            ResultingImgNumModesPtr[index]++;
                        }
                    }

                    if(curprob>maxcurprob){
                        finalint=inten;
                        maxcurprob=curprob;
                    }

                    if(nx==207 && ny==138 && nz==127){
                        cout <<"Intensities:"<< inten << " - "<<maxcurprob<< " - "<<curprob << endl;
                        cout.flush();
                    }
                }

                srand(time(NULL)+oldRand);
                float resVector[5]={0};
                for(size_t i_vec=0; i_vec<5; ++i_vec)
                {
                    resVector[i_vec]=(float)(rand())/(float)RAND_MAX;
                    if(nx==207 && ny==138 && nz==127){
                        cout <<"Rand:"<<i_vec<<" - "<< resVector[i_vec] << endl;
                        cout.flush();
                    }

                }
                oldRand=rand();

                float last2[2]={0,0};
                for(float inten=minIntenLocal; inten<maxIntenLocal; inten+=stepsizeLocal){
                    float curprob=0;
                    for(unsigned char sortlab=0; sortlab<this->sortMaxItems; sortlab++)
                    {
                        if(intenWeight[sortlab]>0 && isnan(intenVector[sortlab])==0){
                            curprob+=intenWeight[sortlab]*expf(-pow((intenVector[sortlab]-inten),2)/(0.5*stdsizeLocal/50));
                        }
                    }
                    last2[0]=last2[1];
                    last2[1]+=curprob;

                    for(size_t i_vec=0; i_vec<5; ++i_vec)
                    {
                        if((last2[1]/totalSumProb)>resVector[i_vec] && (last2[0]/totalSumProb)<=resVector[i_vec]){
                            ResultingImgSamplesPtr[index+i_vec*ResultingImgMAP->nvox]=inten;
                        }
                        if(nx==207 && ny==138 && nz==127){
                            cout <<"Tests:"<<i_vec<<" - "<< resVector[i_vec]<<" - "<< (last2[1]/totalSumProb) <<" - "<< (last2[0]/totalSumProb) <<endl;
                            cout.flush();
                        }

                    }
                }
                ResultingImgMapPtr[index]=finalint;
            }
        }
    }


    ResultingImgMAP->scl_inter=atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImgExpec->scl_inter=atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImgSamples->scl_inter=atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    float min=FLT_MAX;
    float max=-FLT_MAX;
    for(long i=0; i<(long)(ResultingImgMAP->nvox); i++)
    {
        max=ResultingImgMapPtr[i]>max?ResultingImgMapPtr[i]:max;
        min=ResultingImgMapPtr[i]<min?ResultingImgMapPtr[i]:min;
    }
    ResultingImgMAP->cal_min=min+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImgMAP->cal_max=max+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    nifti_image_write(ResultingImgMAP);

    ResultingImgExpec->cal_min=min+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImgExpec->cal_max=max+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    nifti_image_write(ResultingImgExpec);

    ResultingImgSamples->cal_min=min+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    ResultingImgSamples->cal_max=max+atof(this->infoDB[databasenumb]->getExtraFiles()[0].c_str());
    nifti_image_write(ResultingImgSamples);

    ResultingImgNumModes->cal_min=0;
    ResultingImgNumModes->cal_max=3;
    ResultingImgExpec->scl_inter=0;
    nifti_image_write(ResultingImgNumModes);

}

void GIF::processAndSaveFlowAsLike(unsigned int databasenumb)
{
    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Inten Information: "<<filenameStr<< endl;


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> Result = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImg = Result->getPtr();
    ResultingImg->dim[0]=ResultingImg->ndim=4;
    ResultingImg->dim[4]=ResultingImg->nt=1;
    ResultingImg->dim[5]=ResultingImg->nu=1;
    ResultingImg->pixdim[5]=ResultingImg->du=1;
    nifti_update_dims_from_array(ResultingImg);
    ResultingImg->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImg->nbyper = sizeof(float);
    ResultingImg->data = (void *)malloc(ResultingImg->nvox * ResultingImg->nbyper);
    if(ResultingImg->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    nifti_set_filenames(ResultingImg,filenameStr.c_str(),0,0);

    float * ResultingImgPtr=static_cast<float *>(ResultingImg->data);

    //    for(int indext=0; indext<2; indext++)
    //    {
    for(int index=0; index<(ResultingImg->nx*ResultingImg->ny*ResultingImg->nz); index++)
    {
        if(this->targetMask[index]){
            ResultingImgPtr[index]=this->resultsInfoDB[databasenumb]->getValue(index,0)/this->resultsInfoDB[databasenumb]->getValue(index,1);
        }
        else{
            ResultingImgPtr[index]=0;
        }
    }
    //    }
    nifti_image_write(ResultingImg);

    //string filenameGeoStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+"_geo.nii.gz";
    //if(this->getVerbose()>0)
    //    cout << "    - Saving Inten Geo Information: "<<filenameGeoStr<< endl;
    //this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
    //nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());


}

void GIF::processAndSaveFlowAsVel(unsigned int databasenumb)
{
    // Check if the database had any data
    int datasetCount=0;
    for (int i = 0; i<this->infoDB[databasenumb]->getNumberOfDatasets(); i++)
    {
        datasetCount+=(this->infoDB[databasenumb]->getDatasetNoDataPtr(i)==NULL)?0:1;
    }
    if(datasetCount<1){
        cout << "    - Skipping " <<this->infoDB[databasenumb]->getFname()<< " as this database was empty."<<endl;
        return;
    }

    // Filename
    string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
    string filenameStr=this->resultsPath+SEP+strippedTargetFileName+"_"+this->infoDB[databasenumb]->getFname()+".nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Inten Information: "<<filenameStr<< endl;


    // Allocate memory and resample floating into the target space
    smartPointer<niftiObject> Result = new niftiObject(nifti_copy_nim_info(this->getDatabaseTarget(databasenumb)));
    nifti_image *ResultingImg = Result->getPtr();
    ResultingImg->dim[0]=ResultingImg->ndim=5;
    ResultingImg->dim[4]=ResultingImg->nt=1;
    ResultingImg->dim[5]=ResultingImg->nu=3;
    ResultingImg->pixdim[5]=ResultingImg->du=1;
    nifti_update_dims_from_array(ResultingImg);
    ResultingImg->datatype = NIFTI_TYPE_FLOAT32;
    ResultingImg->nbyper = sizeof(float);
    ResultingImg->data = (void *)malloc(ResultingImg->nvox * ResultingImg->nbyper);
    if(ResultingImg->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    nifti_set_filenames(ResultingImg,filenameStr.c_str(),0,0);

    float * ResultingImgPtr=static_cast<float *>(ResultingImg->data);

    for(unsigned char axis=0; axis<3; axis++)
    {
        for(int index=0; index<(ResultingImg->nx*ResultingImg->ny*ResultingImg->nz); index++)
        {
            ResultingImgPtr[index+(axis)*((ResultingImg->nx*ResultingImg->ny*ResultingImg->nz))]=this->resultsInfoDB[databasenumb]->getValue(index,axis)/this->resultsInfoDB[databasenumb]->getValue(index,3);

        }
    }


    reg_getDeformationFromDisplacement(ResultingImg);
    ResultingImg->intent_code=NIFTI_INTENT_VECTOR;
    ResultingImg->intent_p1=DEF_VEL_FIELD; //DISP_VEL_FIELD

    ResultingImg->intent_p2=6;
    for(size_t subjectnumb=0; subjectnumb<this->rootdb->getNumberOfDatasets(); subjectnumb++)
    {
        if(ResultingImg->intent_p2<this->cppdb->getDatasetNoDataPtr(subjectnumb)->intent_p2){
            ResultingImg->intent_p2=this->cppdb->getDatasetNoDataPtr(subjectnumb)->intent_p2;
        }
        this->cppdb->unloadDataFromDataset(subjectnumb);
    }


    nifti_image_write(ResultingImg);
    return;

}

void GIF::PropagateAllSubjectsInformation()
{
    // For each subject, if CPP exists, then propagate information
    if(this->getVerbose()>0)
        cout<<"\n___________________________\n"<<endl;


    for(size_t subjectnumb=0; subjectnumb<this->rootdb->getNumberOfDatasets(); subjectnumb+=1)
    {
        if(this->cppdb->getDatasetNoDataPtr(subjectnumb)!=NULL)
        {
            this->PropagateSubjectInformation(subjectnumb);
            this->cppdb->unloadDataFromDataset(subjectnumb);
        }
        this->freeMemory();
    }
}

void GIF::PropagateSubjectInformation(unsigned int subnumb)
{

    if(this->getVerbose()>0)
    {
        cout << " Similarity "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: ";
        cout<<this->rootdb->getDatasetName(subnumb)<<"\n -> Target: "<<this->target->getPtr()->fname<<endl;
    }

    // Get deformation field and unload ccp from memory
    smartPointer<niftiObject> DeformationField=new niftiObject(this->createDeformationFieldFromSubjectToGeneralTarget(subnumb));

    // Get similarity image
    smartPointer<niftiObject> SimilarityImage=new niftiObject(this->createDistanceFromSubject(subnumb, DeformationField->getPtr()));

    for(size_t databasenumb=0; databasenumb<this->infoDB.size(); databasenumb++)
    {
        this->informationFlow(databasenumb,subnumb,DeformationField->getPtr(),SimilarityImage->getPtr());
        this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);
        this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);
        this->rootdb->unloadDataFromDataset(subnumb);
    }
}

void GIF::informationFlow(unsigned int databasenumb,
                          unsigned int subnumb,
                          nifti_image * DeformationField,
                          nifti_image * DistanceImage)
{

    switch(this->infoDB[databasenumb]->getType())
    {
    case LABEL:
        this->informationFlowLabel(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case PROB:
        this->informationFlowProb(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case LABELSEG:
        this->informationFlowLabel(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case PROBSEG:
        this->informationFlowProb(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case INTEN:
        this->informationFlowInten(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case INTENSORT:
        this->informationFlowIntenSort(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    case INTENMAP:
        this->informationFlowIntenMAP(databasenumb,subnumb,DeformationField);
        break;
    case LIKE:
        this->informationFlowLikelihood(databasenumb,subnumb,DeformationField);
        break;
    case VEL:
        this->informationFlowVelocityField(databasenumb,subnumb,DeformationField,DistanceImage);
        break;
    default:
        cout<<"Invalid Database Type: "<< this->infoDB[databasenumb]->getType();
        exit(1);
        break;
    }
}

void GIF::informationFlowLabel(unsigned int databasenumb,
                               unsigned int subnumb,
                               nifti_image * DeformationField,
                               nifti_image * DistanceImage)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL)
        return;

    if(this->getVerbose()>0)
        cout << "    - Resampling Label Information"<< endl;

    // Allocate memory and resample
    smartPointer<niftiObject> ResampledDistance = new niftiObject(createResampledImageUsingSForm(DistanceImage,this->getDatabaseTarget(databasenumb),1));
    float * DistanceImageResampledPtr=static_cast<float *>(ResampledDistance->getPtr()->data);

    //Deformation Field Composed and Resample image to database target space.
    smartPointer<niftiObject> DeformationFieldComposed = new niftiObject(createDeformationFieldFromGlobalTargetToDatabaseTarget(DeformationField,databasenumb));
    smartPointer<niftiObject> ResampledLabels = new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                          this->getDatabaseTarget(databasenumb),
                                                                                                          DeformationFieldComposed->getPtr(),0));

    // if the current template has a GeoDistance, then estimate D^2+G^2
    // Otherwise, do D^2
    if(this->geoDB[databasenumb].cPTR()!=NULL && this->geoDB[databasenumb]->getDatasetNoDataPtr(subnumb)!=NULL)
    {
        smartPointer<niftiObject> GeoDistance = new niftiObject(createResampledImageUsingDeformationField(this->geoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                          this->getDatabaseTarget(databasenumb),
                                                                                                          DeformationFieldComposed->getPtr(),1));

        float * GeoDistancePtr = static_cast<float*> (GeoDistance->getPtr()->data);
        // Add GeoDistance to the Pairwise Distance
        for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
        {
            DistanceImageResampledPtr[index]=(isnan(GeoDistancePtr[index])==0)?
                        (DistanceImageResampledPtr[index]+GeoDistancePtr[index]):
                        DistanceImageResampledPtr[index];
        }

    }
    // Update resulting GeoDistance with current estimate

    this->resultsGeoDB[databasenumb]->addIfSmaller(ResampledDistance->getPtr());

    // Transform Geodistance into a weight
    for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
    {
        DistanceImageResampledPtr[index]=exp(-DistanceImageResampledPtr[index]/this->temperature);
    }

    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);
    this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);
    int * ROImask=new int [ResampledLabels->getPtr()->nvox]();
    float * ResampledLabelsPtr=static_cast<float *>(ResampledLabels->getPtr()->data);
    for(size_t index=0; index<ResampledLabels->getPtr()->nvox; index++)
    {
        ROImask[index]=1;
        if(isnan(DistanceImageResampledPtr[index])==1)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        if(isnan(ResampledLabelsPtr[index]))  // or if it is nan after resampling
        {
            ROImask[index]=0;
        }
    }

    if(this->getVerbose()>0)
        cout << "    - Flowing Label Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;

    int nvol=ResampledLabels->getPtr()->nx*ResampledLabels->getPtr()->ny*ResampledLabels->getPtr()->nz;
    for(int index=0; index<nvol; index++)
    {
        if(ROImask[index] && isnan(ResampledLabelsPtr[index])==0 && isnan(DistanceImageResampledPtr[index])==0 && DistanceImageResampledPtr[index]>0)
            this->resultsInfoDB[databasenumb]->addToPair(index,(unsigned char)(round(ResampledLabelsPtr[index])),DistanceImageResampledPtr[index]);
    }


    delete [] ROImask;
}


void GIF::informationFlowProb(unsigned int databasenumb,
                              unsigned int subnumb,
                              nifti_image * DeformationField,
                              nifti_image * DistanceImage)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL)
        return;

    if(this->getVerbose()>0)
        cout << "    - Resampling Probabilistic Information"<< endl;

    // Allocate memory and resample
    smartPointer<niftiObject> ResampledDistance =  new niftiObject(createResampledImageUsingSForm(DistanceImage,this->getDatabaseTarget(databasenumb),1));
    //Deformation Field Composed and Resample image to database target space.
    smartPointer<niftiObject> DeformationFieldComposed =  new niftiObject(createDeformationFieldFromGlobalTargetToDatabaseTarget(DeformationField,databasenumb));
    smartPointer<niftiObject> ResampledProb =  new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                         this->getDatabaseTarget(databasenumb),
                                                                                                         DeformationFieldComposed->getPtr(),1));
    float * ResampledProbPtr=static_cast<float *>(ResampledProb->getPtr()->data);

    // if the current template has a GeoDistance, then add to the current distance
    // Otherwise, do nothing (geodist=0)
    if(this->geoDB[databasenumb].cPTR()!=NULL && this->geoDB[databasenumb]->getDatasetNoDataPtr(subnumb)!=NULL)
    {
        cout<<"herehere"<<endl;
        smartPointer<niftiObject> GeoDistance = new niftiObject(createResampledImageUsingDeformationField(this->geoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                          this->getDatabaseTarget(databasenumb),
                                                                                                          DeformationFieldComposed->getPtr(),1));

        float * GeoDistancePtr = static_cast<float*> (GeoDistance->getPtr()->data);
        // Add GeoDistance to the Pairwise Distance
        float * ResampledDistancePtr=static_cast<float *>(ResampledDistance->getPtr()->data);
        for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
        {
            if( isnan(GeoDistancePtr[index])==0 && isnan(ResampledDistancePtr[index])==0 )
            {
                ResampledDistancePtr[index]=ResampledDistancePtr[index]+GeoDistancePtr[index];
            }
        }
        this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);

    }
    // Update resulting GeoDistance with current estimate

    this->resultsGeoDB[databasenumb].cPTR()->addIfSmaller(ResampledDistance->getPtr());
    // Transform Geodistance into a weight

    float * DistanceImageResampledPtr=static_cast<float *>(ResampledDistance->getPtr()->data);
    for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
    {
        DistanceImageResampledPtr[index]=exp(-DistanceImageResampledPtr[index]/this->temperature);
    }

    string filenameGeoStr="test_geo.nii.gz";
    if(this->getVerbose()>0)
        cout << "    - Saving Label Geo Information: "<<filenameGeoStr<< endl;
    this->resultsGeoDB[databasenumb]->setFilename(filenameGeoStr);
    nifti_image_write(this->resultsGeoDB[databasenumb]->getPtr());

    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);

    // Calculate ROI including field of view
    float * SimilarityImagePtr=static_cast<float *>(ResampledDistance->getPtr()->data);
    size_t numvox=ResampledProb->getPtr()->nx*ResampledProb->getPtr()->ny*ResampledProb->getPtr()->nz;
    int * ROImask=new int [numvox]();
    for(size_t index=0; index<numvox; index++)
    {
        ROImask[index]=1;
        if(isnan(SimilarityImagePtr[index])==1)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        if(isnan(ResampledProbPtr[index]))  // or if it is nan after resampling
        {
            ROImask[index]=0;
        }
    }

    if(this->getVerbose()>0)
        cout << "    - Flowing Probabilistic Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;

    int volsize3d=ResampledProb->getPtr()->nx*ResampledProb->getPtr()->ny*ResampledProb->getPtr()->nz;
    int volnt=ResampledProb->getPtr()->nt;

    for(int index=0; index<volsize3d; index++)
    {
        if(ROImask[index])
        {
            for(int nlab=0; nlab<volnt; nlab++)
            {
                if(isnan(ResampledProbPtr[index+nlab*volsize3d])==0 && DistanceImageResampledPtr[index]>0 && ResampledProbPtr[index+nlab*volsize3d]>0 && isnan(DistanceImageResampledPtr[index])==0 )
                {
                    this->resultsInfoDB[databasenumb]->addToPair(index,nlab,DistanceImageResampledPtr[index]*ResampledProbPtr[index+nlab*volsize3d]);
                }
            }
        }
    }

    delete [] ROImask;
}


void GIF::informationFlowInten(unsigned int databasenumb,
                               unsigned int subnumb,
                               nifti_image * DeformationField,
                               nifti_image * DistanceImage)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL){
        return;
    }

    // Allocate memory and resample floating into the target space
    if(this->getVerbose()>0)
        cout << "    - Resampling Intensity Information"<< endl;



    // The Similarity image is in the space of the general target.
    // Needs to be resampled to the database target using only the sform
    smartPointer<niftiObject> ResampledDistance = new niftiObject(createResampledImageUsingSForm(DistanceImage,this->getDatabaseTarget(databasenumb),1));
    float * DistanceImageResampledPtr=static_cast<float *>(ResampledDistance->getPtr()->data);

    // The Atlas image is in its original space.
    // Needs to be resampled to the database target using:
    // - the deformation field (atlas to general target)
    // - the sform (general target to database target)
    // This is acheived through composition.
    smartPointer<niftiObject> DeformationFieldComposed = new niftiObject(createDeformationFieldFromGlobalTargetToDatabaseTarget(DeformationField,databasenumb));
    smartPointer<niftiObject> ResampledImage = new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                         this->getDatabaseTarget(databasenumb),
                                                                                                         DeformationFieldComposed->getPtr(),3));

    // if the current template has a GeoDistance, then add to the current distance
    // Otherwise, do nothing (geodist=0)
    if(this->geoDB[databasenumb].cPTR()!=NULL && this->geoDB[databasenumb]->getDatasetNoDataPtr(subnumb)!=NULL)
    {
        smartPointer<niftiObject> GeoDistance = new niftiObject(createResampledImageUsingDeformationField(this->geoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                          this->getDatabaseTarget(databasenumb),
                                                                                                          DeformationFieldComposed->getPtr(),1));

        float * GeoDistancePtr = static_cast<float*> (GeoDistance->getPtr()->data);

        // Add GeoDistance to the Pairwise Distance
        for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
        {
            DistanceImageResampledPtr[index]=(isnan(GeoDistancePtr[index])==0)?(DistanceImageResampledPtr[index]+GeoDistancePtr[index]):DistanceImageResampledPtr[index];
        }

    }

    // Update resulting GeoDistance with current estimate
    this->resultsGeoDB[databasenumb]->addIfSmaller(ResampledDistance->getPtr());


    // Transform Geodistance into a weight
    for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
    {
        DistanceImageResampledPtr[index]=exp(-DistanceImageResampledPtr[index]/this->temperature);
    }

    // Clear up some unnecessary memory
    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);
    this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);

    // Set the ROI region (i.e. the region wich is within the general target mask and within the image edges)
    float * ResampledImagePtr=static_cast<float *>(ResampledImage->getPtr()->data);
    size_t numvox=ResampledImage->getPtr()->nvox;
    int * ROImask=new int [numvox]();
    for(size_t index=0; index<numvox; index++)
    {
        ROImask[index]=1;
        if(isnan(DistanceImageResampledPtr[index])==1)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        if(isnan(ResampledImagePtr[index])==1)  // or if it is nan after resampling
        {
            ROImask[index]=0;
        }
    }

    // Actually do the information flow part just by adding to the image structure
    // sum(I*W) will be stored under label 1 and sum(W) will be stored under label 0
    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;
    for(size_t index=0; index<numvox; index++)
    {
        if(ROImask[index] && DistanceImageResampledPtr[index]>0)
        {
            this->resultsInfoDB[databasenumb]->addToPair(index,1,ResampledImagePtr[index]*DistanceImageResampledPtr[index]);
            this->resultsInfoDB[databasenumb]->addToPair(index,0,DistanceImageResampledPtr[index]);
        }
    }

    // Clean the house
    delete [] ROImask;

    return;
}


void GIF::informationFlowIntenSort(unsigned int databasenumb,
                                   unsigned int subnumb,
                                   nifti_image * DeformationField,
                                   nifti_image * DistanceImage)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL){
        return;
    }

    // Allocate memory and resample floating into the target space
    if(this->getVerbose()>0)
        cout << "    - Resampling Intensity Information"<< endl;



    // The Similarity image is in the space of the general target.
    // Needs to be resampled to the database target using only the sform
    smartPointer<niftiObject> ResampledDistance = new niftiObject(createResampledImageUsingSForm(DistanceImage,this->getDatabaseTarget(databasenumb),1));
    float * DistanceImageResampledPtr=static_cast<float *>(ResampledDistance->getPtr()->data);

    // The Atlas image is in its original space.
    // Needs to be resampled to the database target using:
    // - the deformation field (atlas to general target)
    // - the sform (general target to database target)
    // This is acheived through composition.
    smartPointer<niftiObject> DeformationFieldComposed = new niftiObject(createDeformationFieldFromGlobalTargetToDatabaseTarget(DeformationField,databasenumb));

    smartPointer<niftiObject> ResampledImage = new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                         this->getDatabaseTarget(databasenumb),
                                                                                                         DeformationFieldComposed->getPtr(),3));

    // if the current template has a GeoDistance, then add to the current distance
    // Otherwise, do nothing (geodist=0)
    if(this->geoDB[databasenumb].cPTR()!=NULL && this->geoDB[databasenumb]->getDatasetNoDataPtr(subnumb)!=NULL)
    {
        smartPointer<niftiObject> GeoDistance = new niftiObject(createResampledImageUsingDeformationField(this->geoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                          this->getDatabaseTarget(databasenumb),
                                                                                                          DeformationFieldComposed->getPtr(),1));

        float * GeoDistancePtr = static_cast<float*> (GeoDistance->getPtr()->data);

        // Add GeoDistance to the Pairwise Distance
        for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
        {
            DistanceImageResampledPtr[index]=(isnan(GeoDistancePtr[index])==0)?
                        (DistanceImageResampledPtr[index]+GeoDistancePtr[index]):
                        DistanceImageResampledPtr[index];
        }

    }

    // Update resulting GeoDistance with current estimate

    this->resultsGeoDB[databasenumb]->addIfSmaller(ResampledDistance->getPtr());


    // The sorting method only does this after the propagation
    //    // Transform Geodistance into a weight
    //    for(size_t index=0; index<ResampledDistance->getPtr()->nvox; index++)
    //    {
    //        DistanceImageResampledPtr[index]=exp(-DistanceImageResampledPtr[index]/this->temperature);
    //    }

    // Clear up some unnecessary memory
    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);
    this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);

    // Set the ROI region (i.e. the region wich is within the general target mask and within the image edges)
    float * ResampledImagePtr=static_cast<float *>(ResampledImage->getPtr()->data);
    size_t numvox=ResampledImage->getPtr()->nvox;
    int * ROImask=new int [numvox]();
    for(size_t index=0; index<numvox; index++)
    {
        ROImask[index]=1;
        if(isnan(DistanceImageResampledPtr[index])==1)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        if(isnan(ResampledImagePtr[index])==1)  // or if it is nan after resampling
        {
            ROImask[index]=0;
        }
    }

    // Actually do the information flow part by calling the hacky assignSortToPair function.
    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;
    for(size_t index=0; index<numvox; index++)
    {
        if(ROImask[index] && DistanceImageResampledPtr[index]>0)
        {

            this->resultsInfoDB[databasenumb]->assignSortToPair(index,this->sortMaxItems,ResampledImagePtr[index],DistanceImageResampledPtr[index]);
        }
    }

    // Clean the house
    delete [] ROImask;

    return;
}


void GIF::informationFlowLikelihood(unsigned int databasenumb,
                                    unsigned int subnumb,
                                    nifti_image * DeformationField)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL){
        return;
    }

    // Allocate memory and resample floating into the target space
    if(this->getVerbose()>0)
        cout << "    - Resampling Intensity Information"<< endl;

    long shiftspacing=1;
    long shiftsize=0; //Neighbourhood size = shiftsize*2+1

    int patchsizeX=3;
    //    int patchnumelementsX=pow((float)patchsizeX,3.0f);
    int patchsizeY=3;
    //    int patchnumelementsY=pow((float)patchsizeY,3.0f);

    int directioncount=0;


    // The Atlas image is in its original space.
    // Needs to be resampled to the database target using:
    // - the deformation field (atlas to general target)
    // - the sform (general target to database target)
    // This is acheived through composition.

    smartPointer<niftiObject> ResampledRootImage = new niftiObject(createResampledImageUsingDeformationField(this->rootdb->getDatasetPtr(subnumb),
                                                                                                             this->getDatabaseTarget(databasenumb),
                                                                                                             DeformationField,3));
    smartPointer<niftiObject> ResampledInfoImage = new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                             this->getDatabaseTarget(databasenumb),
                                                                                                             DeformationField,3));


    nifti_image * LaplacianImg=createLaplacian();
    float * Laplacian=static_cast<float *>(LaplacianImg->data);
    // Clear up some unnecessary memory
    this->rootdb->unloadDataFromDataset(subnumb);
    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);

    // Set the ROI region (i.e. the region wich is within the general target mask and within the image edges)
    float * ResampledRootImagePtr=static_cast<float *>(ResampledRootImage->getPtr()->data);
    float * ResampledInfoImagePtr=static_cast<float *>(ResampledInfoImage->getPtr()->data);
    long numvox=ResampledRootImage->getPtr()->nvox;
    long index=0;
    int * ROImask=new int [numvox]();
    for(index=0; index<numvox; index++)
    {
        // if it is outside the subject mask or if it is nan after resampling
        ROImask[index]=(this->targetMask[index]==0 || isnan(ResampledRootImagePtr[index])==1 || isnan(ResampledInfoImagePtr[index])==1 )?0:1;
    }

    // Actually do the information flow part just by normalising and estimating the likelihood
    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information and estiamting the likelihood: "<<this->infoDB[databasenumb]->getDescription()<< endl;

    normaliseImage(ResampledRootImage->getPtr(), this->target->getPtr(), ROImask);
    normaliseImage(ResampledInfoImage->getPtr(), this->getDatabaseTarget(databasenumb), ROImask);

    float * RootTargetPtr=static_cast<float *>(this->target->getPtr()->data);
    float * InfoTargetPtr=static_cast<float *>(this->getDatabaseTarget(databasenumb)->data);

    if(this->getVerbose()>0)
        cout << "    - Estimating Noise"<< endl;

    double calc=0;
    double calccount=0;
    for(unsigned int index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            calc += (RootTargetPtr[index]-ResampledRootImagePtr[index]);
            calccount+=1;
        }
    }
    double mean=(double)(calc)/(double)(calccount);
    calc=0;
    calccount=0;
    for(unsigned int index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            calc += powf(mean-(RootTargetPtr[index]-ResampledRootImagePtr[index]),2);
            calccount+=1;
        }
    }

    double globalstd=sqrt((double)(calc)/(double)(calccount));
    cout<<globalstd<<endl;


    float * ImageMean=new float [numvox];
    float * ImageStd=new float [numvox];
    for(index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            ImageMean[index]=(double)(RootTargetPtr[index]-ResampledRootImagePtr[index]);
            ImageStd[index]=(double)(RootTargetPtr[index]-ResampledRootImagePtr[index])*(double)(RootTargetPtr[index]-ResampledRootImagePtr[index]);
        }
        else{
            ImageMean[index]=std::numeric_limits<float>::quiet_NaN();
            ImageStd[index]=std::numeric_limits<float>::quiet_NaN();
        }
    }
    // Create a dummy nifty buffer for the gaussian blurring
    nifti_image *GaussianBuffer = nifti_copy_nim_info(this->target->getPtr());
    GaussianBuffer->dim[0]=GaussianBuffer->ndim=3;
    GaussianBuffer->dim[5]=GaussianBuffer->nu=1;
    GaussianBuffer->pixdim[5]=GaussianBuffer->du=1;
    nifti_update_dims_from_array(GaussianBuffer);
    GaussianBuffer->datatype = NIFTI_TYPE_FLOAT32;
    GaussianBuffer->nbyper = sizeof(float);

    GaussianBuffer->data=ImageMean;
    GaussianSmoothing5D_nifti(GaussianBuffer,NULL,patchsizeX);///2.634f);
    GaussianBuffer->data=ImageStd;
    GaussianSmoothing5D_nifti(GaussianBuffer,NULL,patchsizeX);///2.634f);
    GaussianBuffer->data=NULL;
    nifti_image_free(GaussianBuffer);
    for(index=0; index<numvox; index++)
    {
        ImageStd[index]=ROImask[index] ? sqrt(ImageStd[index]-ImageMean[index]*ImageMean[index]):std::numeric_limits<float>::quiet_NaN();
    }
    delete [] ImageMean;


    // Estimate the L2norm on a local region
    float * buffer1=new float [numvox];
    float * buffer1up=new float [numvox];
    float * buffer2=new float [numvox];
    float * buffer2up=new float [numvox];
    //float * bufferNormaliser=new float [numvox];

    if(buffer1==NULL && buffer1up==NULL && buffer2==NULL && buffer2up==NULL)
    {
        cout<<"ERROR: Bad Allocation of the buffers"<<endl;
        exit(1);
    }


    // Create a fake object for the kernel
    smartPointer<niftiObject> Buffer=new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    Buffer->getPtr()->dim[0]=Buffer->getPtr()->ndim=3;
    Buffer->getPtr()->dim[4]=Buffer->getPtr()->nt=1;
    Buffer->getPtr()->dim[5]=Buffer->getPtr()->nu=1;
    Buffer->getPtr()->pixdim[5]=Buffer->getPtr()->du=1;
    nifti_update_dims_from_array(Buffer->getPtr());
    Buffer->getPtr()->datatype = NIFTI_TYPE_FLOAT32;
    Buffer->getPtr()->nbyper = sizeof(float);


#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,buffer1up,buffer2,buffer2up,numvox) \
    private(index)
#endif
    for(long index=0; index<numvox; index++)
    {
        buffer1[index]=buffer1up[index]=buffer2[index]=buffer2up[index]=0;
    }

    const long dims[3]={(long)(this->target->getPtr()->dim[1]),
                        (long)(this->target->getPtr()->dim[2]),
                        (long)(this->target->getPtr()->dim[3])};
    int * targetmaskptr=this->targetMask;

    // Kernel Shift
    for(long shiftx=-shiftsize; shiftx<=shiftsize; shiftx++){
        for(long shifty=-shiftsize; shifty<=shiftsize; shifty++){
            for(long shiftz=-shiftsize; shiftz<=shiftsize; shiftz++){

                cout<<directioncount<<endl;
                directioncount++;
                long index=0;
                long index2=0;
                long nx=0;
                long ny=0;
                long nz=0;

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,buffer2,RootTargetPtr,ResampledRootImagePtr,InfoTargetPtr,ResampledInfoImagePtr,targetmaskptr,dims,numvox,shiftx,shifty,shiftz,shiftspacing) \
    private(index,index2,nx,ny,nz)
#endif
                for(nx=0; nx<(long)dims[0]; nx++)
                {
                    for(ny=0; ny<(long)dims[1]; ny++)
                    {
                        for(nz=0; nz<(long)dims[2]; nz++)
                        {
                            index=nx+dims[0]*(ny+(dims[1]*nz));
                            index2=(nx+shiftx*shiftspacing)+dims[0]*((ny+shifty*shiftspacing)+(dims[1]*(nz+shiftz*shiftspacing)));

                            if(targetmaskptr[index]==1   && // Is within the mask
                                    index2<(long)(numvox)   && // Is within the index bounds (also checks z)
                                    index2>=0               && // Is within the index bounds (also checks z)
                                    (nx+shiftx) < dims[0]   && // Does it flip around the image
                                    (nx+shiftx) >= 0        && // Does it flip around the image
                                    (ny+shifty) < dims[1]   && // Does it flip around the image
                                    (ny+shifty) >= 0 )         // Does it flip around the image
                            {

                                buffer1[index]=RootTargetPtr[index]-ResampledRootImagePtr[index2];
                                buffer1[index]*=buffer1[index]; // pow2
                                buffer2[index]=InfoTargetPtr[index]-ResampledInfoImagePtr[index2];
                                buffer2[index]*=buffer2[index]; // pow2
                            }
                            else{
                                buffer1[index]=std::numeric_limits<float>::quiet_NaN();
                                buffer2[index]=std::numeric_limits<float>::quiet_NaN();
                            }
                        }
                    }
                }

                Buffer->getPtr()->data=buffer1;
                BlockSmoothing(Buffer->getPtr(),targetmaskptr,patchsizeX);
                Buffer->getPtr()->data=buffer2;
                BlockSmoothing(Buffer->getPtr(),targetmaskptr,patchsizeY);
                Buffer->getPtr()->data=NULL;

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,buffer2,buffer1up,buffer2up,targetmaskptr,dims,numvox,ImageStd,globalstd,shiftx,shifty,shiftz,shiftspacing,Laplacian) \
    private(index)
#endif
                for(index=0; index<numvox; index++)
                {
                    if(targetmaskptr[index] && isnan(buffer1[index])==0 && isnan(buffer2[index])==0)
                    {
                        float vec[3]={(float)(shiftx*shiftspacing),(float)(shifty*shiftspacing),(float)(shiftz*shiftspacing)};

                        float DistanceLikelihood=(vec[0]*(vec[0]*Laplacian[index+0*numvox]+vec[1]*Laplacian[index+1*numvox]+vec[2]*Laplacian[index+2*numvox])+     // xx xy xz - 0 1 2
                                vec[1]*(vec[0]*Laplacian[index+1*numvox]+vec[1]*Laplacian[index+3*numvox]+vec[2]*Laplacian[index+4*numvox])+     // yx yy yz - 1 3 4
                                vec[2]*(vec[0]*Laplacian[index+2*numvox]+vec[1]*Laplacian[index+4*numvox]+vec[2]*Laplacian[index+5*numvox]));   // zx zy zz - 2 4 6

                        // Likelihood of Root
                        buffer2up[index]+=exp(-(buffer2[index])/((ImageStd[index]*globalstd))-(buffer1[index])/((ImageStd[index]*globalstd))-DistanceLikelihood);
                        //buffer2up[index]+=exp(-(buffer2[index])/(ImageStd[index]*ImageStd[index])-(buffer1[index])/(ImageStd[index]*ImageStd[index])-DistanceLikelihood);

                        // Likelihood of Info
                        buffer1up[index]+=expf(-DistanceLikelihood);//(buffer1[index]*patchnumelementsX)/(2.0f*ImageStd[index]));

                        //bufferNormaliser[index]+=DistanceLikelihood;
                    }
                }
            }
        }
    }

    smartPointer<mapImage> currentmap=(this->resultsInfoDB[databasenumb]);

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(currentmap,buffer1up,buffer2up,targetmaskptr,numvox,ImageStd) \
    private(index)
#endif
    for(index=0; index<numvox; index++)
    {
        if(targetmaskptr[index] && isnan(buffer1up[index])==0 && isnan(buffer2up[index])==0  )
        {
            // Likelihood of Root
            currentmap->addToPair(index,0,buffer2up[index]/buffer1up[index]);
            // Likelihood of Info
            currentmap->addToPair(index,1, 1);
            // Likelihood of Root
        }
    }
    // Clean the house
    delete [] buffer1;
    delete [] buffer1up;
    delete [] buffer2;
    delete [] buffer2up;
    //delete [] bufferNormaliser;
    delete [] ROImask;
    delete [] ImageStd;

    return;
}


void GIF::informationFlowIntenMAP(unsigned int databasenumb,
                                  unsigned int subnumb,
                                  nifti_image * DeformationField)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL){
        return;
    }

    // Allocate memory and resample floating into the target space
    if(this->getVerbose()>0)
        cout << "    - Resampling Intensity Information"<< endl;

    long shiftspacing=1;
    long shiftsize=0; //Neighbourhood size = shiftsize*2+1

    int patchsizeX=5 ;
    //    int patchnumelementsX=pow((float)patchsizeX,3.0f);

    int directioncount=0;


    // The Atlas image is in its original space.
    // Needs to be resampled to the database target using:
    // - the deformation field (atlas to general target)
    // - the sform (general target to database target)
    // This is acheived through composition.

    smartPointer<niftiObject> ResampledRootImage = new niftiObject(createResampledImageUsingDeformationField(this->rootdb->getDatasetPtr(subnumb),
                                                                                                             this->getDatabaseTarget(databasenumb),
                                                                                                             DeformationField,3));
    smartPointer<niftiObject> ResampledInfoImage = new niftiObject(createResampledImageUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb),
                                                                                                             this->getDatabaseTarget(databasenumb),
                                                                                                             DeformationField,3));


    //    nifti_image * LaplacianImg=createLaplacian();
    //    float * Laplacian=static_cast<float *>(LaplacianImg->data);
    // Clear up some unnecessary memory
    this->rootdb->unloadDataFromDataset(subnumb);
    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);

    // Set the ROI region (i.e. the region wich is within the general target mask and within the image edges)
    float * ResampledRootImagePtr=static_cast<float *>(ResampledRootImage->getPtr()->data);
    float * ResampledInfoImagePtr=static_cast<float *>(ResampledInfoImage->getPtr()->data);
    long numvox=ResampledRootImage->getPtr()->nvox;
    long index=0;
    int * ROImask=new int [numvox]();
    for(index=0; index<numvox; index++)
    {
        // if it is outside the subject mask or if it is nan after resampling
        ROImask[index]=(this->targetMask[index]==0 || isnan(ResampledRootImagePtr[index])==1 || isnan(ResampledInfoImagePtr[index])==1 )?0:1;
    }

    // Actually do the information flow part just by normalising and estimating the likelihood
    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information and estiamting the likelihood: "<<this->infoDB[databasenumb]->getDescription()<< endl;

    normaliseImage(ResampledRootImage->getPtr(), this->target->getPtr(), ROImask);
    //normaliseImage(ResampledInfoImage->getPtr(), this->getDatabaseTarget(databasenumb), ROImask);

    float * RootTargetPtr=static_cast<float *>(this->target->getPtr()->data);


    if(this->getVerbose()>0)
        cout << "    - Estimating Noise"<< endl;

    double calc=0;
    double calccount=0;
    for(unsigned int index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            calc += (RootTargetPtr[index]-ResampledRootImagePtr[index]);
            calccount+=1;
        }
    }
    double mean=(double)(calc)/(double)(calccount);
    calc=0;
    calccount=0;
    for(unsigned int index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            calc += powf(mean-(RootTargetPtr[index]-ResampledRootImagePtr[index]),2);
            calccount+=1;
        }
    }

    double globalstd=sqrt((double)(calc)/(double)(calccount));





    // Estimate the L2norm on a local region
    float * buffer1=new float [numvox];
    float * buffer1up=new float [numvox];
    float * buffer2up=new float [numvox];
    //float * bufferNormaliser=new float [numvox];

    if(buffer1==NULL && buffer1up==NULL && buffer2up==NULL)
    {
        cout<<"ERROR: Bad Allocation of the buffers"<<endl;
        exit(1);
    }


    // Create a fake object for the kernel
    smartPointer<niftiObject> Buffer=new niftiObject(nifti_copy_nim_info(this->target->getPtr()));
    Buffer->getPtr()->dim[0]=Buffer->getPtr()->ndim=3;
    Buffer->getPtr()->dim[4]=Buffer->getPtr()->nt=1;
    Buffer->getPtr()->dim[5]=Buffer->getPtr()->nu=1;
    Buffer->getPtr()->pixdim[5]=Buffer->getPtr()->du=1;
    nifti_update_dims_from_array(Buffer->getPtr());
    Buffer->getPtr()->datatype = NIFTI_TYPE_FLOAT32;
    Buffer->getPtr()->nbyper = sizeof(float);


#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,buffer1up,buffer2up,numvox) \
    private(index)
#endif
    for(long index=0; index<numvox; index++)
    {
        buffer1[index]=0;
        buffer1up[index]=buffer2up[index]=0;
    }

    const long dims[3]={(long)(this->target->getPtr()->dim[1]),
                        (long)(this->target->getPtr()->dim[2]),
                        (long)(this->target->getPtr()->dim[3])};
    int * targetmaskptr=this->targetMask;

    // Kernel Shift
    for(long shiftx=-shiftsize; shiftx<=shiftsize; shiftx++){
        for(long shifty=-shiftsize; shifty<=shiftsize; shifty++){
            for(long shiftz=-shiftsize; shiftz<=shiftsize; shiftz++){

                cout<<directioncount<<endl;
                directioncount++;
                long index=0;
                long index2=0;
                long nx=0;
                long ny=0;
                long nz=0;

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,RootTargetPtr,ResampledRootImagePtr,ResampledInfoImagePtr,targetmaskptr,dims,numvox,shiftx,shifty,shiftz,shiftspacing) \
    private(index,index2,nx,ny,nz)
#endif
                for(nx=0; nx<(long)dims[0]; nx++)
                {
                    for(ny=0; ny<(long)dims[1]; ny++)
                    {
                        for(nz=0; nz<(long)dims[2]; nz++)
                        {
                            index=nx+dims[0]*(ny+(dims[1]*nz));
                            index2=(nx+shiftx*shiftspacing)+dims[0]*((ny+shifty*shiftspacing)+(dims[1]*(nz+shiftz*shiftspacing)));

                            if(targetmaskptr[index]==1   && // Is within the mask
                                    index2<(long)(numvox)   && // Is within the index bounds (also checks z)
                                    index2>=0               && // Is within the index bounds (also checks z)
                                    (nx+shiftx*shiftspacing) < dims[0]   && // Does it flip around the image
                                    (nx+shiftx*shiftspacing) >= 0        && // Does it flip around the image
                                    (ny+shifty*shiftspacing) < dims[1]   && // Does it flip around the image
                                    (ny+shifty*shiftspacing) >= 0 )         // Does it flip around the image
                            {

                                buffer1[index]=RootTargetPtr[index]-ResampledRootImagePtr[index2];
                                buffer1[index]*=buffer1[index]; // pow2
                            }
                            else{
                                buffer1[index]=std::numeric_limits<float>::quiet_NaN();
                            }
                        }
                    }
                }
                Buffer->getPtr()->data=buffer1;
                BlockSmoothing(Buffer->getPtr(),targetmaskptr,patchsizeX);
                Buffer->getPtr()->data=NULL;
                float oneOverSqrt2PiVar=1;//(1/(sqrt(2*M_PI)*globalstd));

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(buffer1,buffer1up,buffer2up,ResampledInfoImagePtr,oneOverSqrt2PiVar,targetmaskptr,dims,numvox,globalstd,shiftx,shifty,shiftz,shiftspacing) \
    private(index,index2,nx,ny,nz)
#endif
                for(nx=0; nx<(long)dims[0]; nx++)
                {
                    for(ny=0; ny<(long)dims[1]; ny++)
                    {
                        for(nz=0; nz<(long)dims[2]; nz++)
                        {
                            index=nx+dims[0]*(ny+(dims[1]*nz));
                            index2=index+(shiftx*shiftspacing)+dims[0]*((shifty*shiftspacing)+(dims[1]*(shiftz*shiftspacing)));

                            if(targetmaskptr[index]==1   && // Is within the mask
                                    index2<(long)(numvox)   && // Is within the index bounds (also checks z)
                                    index2>=0               && // Is within the index bounds (also checks z)
                                    (nx+shiftx*shiftspacing) < dims[0]   && // Does it flip around the image
                                    (nx+shiftx*shiftspacing) >= 0        && // Does it flip around the image
                                    (ny+shifty*shiftspacing) < dims[1]   && // Does it flip around the image
                                    (ny+shifty*shiftspacing) >= 0 )         // Does it flip around the image
                            {
                                if(targetmaskptr[index] && isnan(buffer1[index])==0)
                                {

                                    float tmpprob=oneOverSqrt2PiVar*exp(-(buffer1[index])/(0.5*globalstd*globalstd));
                                    if(buffer2up[index]<tmpprob && tmpprob==tmpprob){
                                        buffer2up[index]=tmpprob;
                                        buffer1up[index]=ResampledInfoImagePtr[index2];
                                    }

                                    //bufferNormaliser[index]+=DistanceLikelihood;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;
    for(size_t index=0; index<numvox; index++)
    {
        if(ROImask[index])
        {
            this->resultsInfoDB[databasenumb]->assignSortToPair(index,this->sortMaxItems,buffer1up[index],-buffer2up[index]);
        }
    }


    // Clean the house
    delete [] buffer1;
    delete [] buffer1up;
    delete [] buffer2up;
    //delete [] bufferNormaliser;
    delete [] ROImask;
    //    delete [] ImageStd;
    //    delete [] ImageMean;

    return;
}

void GIF::informationFlowVelocityField(unsigned int databasenumb,
                                       unsigned int subnumb,
                                       nifti_image * DeformationField,
                                       nifti_image * DistanceImage)
{

    if(this->infoDB[databasenumb]->getDatasetNoDataPtr(subnumb)==NULL){
        return;
    }

    // Allocate memory and resample floating into the target space
    if(this->getVerbose()>0)
    {
        cout << "    - Resampling Velocity Field Information"<< endl;
    }

    // The Similarity image is in the space of the general target.
    // Needs to be resampled to the database target using only the sform
    //smartPointer<niftiObject> ResampledDistance = new niftiObject(createResampledImageUsingSForm(DistanceImage,this->getDatabaseTarget(databasenumb),1));
    float * DistanceImagePtr=static_cast<float *>(DistanceImage->data);
    // Transform Geodistance into a weight
    float epsilon=0.001;
    for(size_t index=0; index<DistanceImage->nvox; index++)
    {
        if(isnan(DistanceImagePtr[index])==0){ // check if metric is nan
            if(DistanceImagePtr[index]>0){ //check if metric went crazy
                DistanceImagePtr[index]=exp(-(DistanceImagePtr[index])/this->temperature)+epsilon;
            }
        }
        else{
            DistanceImagePtr[index]=epsilon;
        }
    }








    // The Atlas image is in its original space.
    // Needs to be resampled to the database target using:
    // - the deformation field (atlas to general target)
    // - the sform (general target to database target)
    // This is acheived through composition.
    //  smartPointer<niftiObject> DeformationFieldComposed = new niftiObject(createDeformationFieldFromGlobalTargetToDatabaseTarget(DeformationField,databasenumb));

    // TRANSPORT
    smartPointer<niftiObject> WarpedVelocityField =
            new niftiObject(createTransportVectorUsingDeformationField(this->infoDB[databasenumb]->getDatasetPtr(subnumb), // Velocity grid
                                                                       this->rootdb->getDatasetPtr(subnumb), // Image where the velocity grid is defined
                                                                       this->getDatabaseTarget(databasenumb), // Target image to transport the velocity field
                                                                       DeformationField, // The Defirmation field deforming the rootdb image to the target
                                                                       1)); // resampling using trilinear interpolations


    // Clear up some unnecessary memory
    this->infoDB[databasenumb]->unloadDataFromDataset(subnumb);
    this->geoDB[databasenumb]->unloadDataFromDataset(subnumb);
    this->rootdb->unloadDataFromDataset(subnumb);

    // Set the ROI region (i.e. the region wich is within the general target mask and within the image edges)
    float * WarpedVelocityFieldPtr=static_cast<float *>(WarpedVelocityField->getPtr()->data);
    size_t numvox=(size_t)WarpedVelocityField->getPtr()->nx*(size_t)WarpedVelocityField->getPtr()->ny*(size_t)WarpedVelocityField->getPtr()->nz;
    int * ROImask=new int [numvox]();
    for(long int index=0; index<numvox; index++)
    {
        ROImask[index]=1;
        if(isnan(DistanceImagePtr[index])==1)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        if(isnan(WarpedVelocityFieldPtr[index])==1)  // or if it is nan after resampling
        {
            ROImask[index]=0;
        }
    }

    //    nifti_set_filenames(WarpedVelocityField->getPtr(),(const char *)"warped.nii.gz",0,0);
    //    nifti_image_write(WarpedVelocityField->getPtr());

    // Actually do the information flow part just by adding to the image structure
    // sum(I*W) will be stored under label 1 and sum(W) will be stored under label 0
    if(this->getVerbose()>0)
        cout << "    - Flowing Intensity Information: "<<this->infoDB[databasenumb]->getDescription()<< endl;
    for(size_t index=0; index<numvox; index++)
    {
        if(ROImask[index] &&
                DistanceImagePtr[index]>0 &&
                isnan(DistanceImagePtr[index])==0 &&
                isnan(WarpedVelocityFieldPtr[index])==0 &&
                isnan(WarpedVelocityFieldPtr[index+numvox])==0 &&
                isnan(WarpedVelocityFieldPtr[index+2*numvox])==0
                )
        {
            this->resultsInfoDB[databasenumb]->addToPair(index,0,WarpedVelocityFieldPtr[index]*DistanceImagePtr[index]);  // X
            this->resultsInfoDB[databasenumb]->addToPair(index,1,WarpedVelocityFieldPtr[index+numvox]*DistanceImagePtr[index]); // Y
            this->resultsInfoDB[databasenumb]->addToPair(index,2,WarpedVelocityFieldPtr[index+2*numvox]*DistanceImagePtr[index]); // Z
            this->resultsInfoDB[databasenumb]->addToPair(index,3,DistanceImagePtr[index]);
        }
    }

    // Clean the house
    delete [] ROImask;

    return;
}



// Gets the pointer to the database target image
// If the database target is nor defined, then use the global target
nifti_image * GIF::getDatabaseTarget(unsigned int databasenumb)
{
    if(this->infoDB[databasenumb]->getTargetPtr()==NULL)
    {
        return this->target->getPtr();
    }
    else
    {
        return this->infoDB[databasenumb]->getTargetPtr();
    }
}

nifti_image * GIF::createTargetMask()
{
    nifti_image * TmpMask = nifti_copy_nim_info((const nifti_image *)this->target->getPtr());
    TmpMask->data = (void *)calloc(this->target->getPtr()->nvox, sizeof(float));
    nifti_set_filenames(TmpMask,"./Mask.nii.gz",0,0);
    if(TmpMask->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate Mask"<< endl;
        exit(1);
    }
    float * Maskprt=static_cast<float *>(TmpMask->data);

    size_t tmpnvox=TmpMask->nvox;
    long int index=0;
    int * targetmaskptr=this->targetMask;


#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(Maskprt,targetmaskptr,tmpnvox) \
    private(index)
#endif
    for(index=0; index<tmpnvox; index++)
    {
        Maskprt[index]=targetmaskptr[index]>0;
    }
    return TmpMask;

}

// Resample the image "ImageToResample" to the space of "TargetImage" using a deformation field "DeformationField"
nifti_image * GIF::createResampledImageUsingDeformationField(nifti_image * ImageToResample, nifti_image * TargetImage, nifti_image * DeformationField, unsigned int typeOfInterpolation)
{

    // Allocate the image being resampled
    nifti_image * ResampledImage=nifti_copy_nim_info(TargetImage);
    ResampledImage->dim[0]=ResampledImage->ndim=ImageToResample->ndim;
    ResampledImage->dim[4]=ResampledImage->nt=ImageToResample->nt;
    ResampledImage->dim[5]=ResampledImage->nu=ImageToResample->nu;
    ResampledImage->pixdim[5]=ResampledImage->du=0;
    nifti_update_dims_from_array(ResampledImage);
    ResampledImage->datatype = NIFTI_TYPE_FLOAT32;
    ResampledImage->nbyper = sizeof(float);
    ResampledImage->data = (void *)malloc(ResampledImage->nvox * ResampledImage->nbyper);
    if(ResampledImage->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Actually resample the image given the deformation field
    reg_resampleImage(ImageToResample,
                      ResampledImage,
                      DeformationField,
                      NULL,
                      typeOfInterpolation,
                      std::numeric_limits<float>::quiet_NaN());
    return ResampledImage;
}

// Resample the image "ImageToResample" to the space of "TargetImage" using a deformation field "DeformationField"
nifti_image * GIF::createResampledImageUsingAffine(nifti_image * ImageToResample, nifti_image * TargetImage, mat44 * InputAffine, unsigned int typeOfInterpolation)
{

    // Allocate the image being resampled
    nifti_image * ResampledImage=nifti_copy_nim_info(TargetImage);
    ResampledImage->dim[0]=ResampledImage->ndim=ImageToResample->ndim;
    ResampledImage->dim[4]=ResampledImage->nt=ImageToResample->nt;
    ResampledImage->dim[5]=ResampledImage->nu=ImageToResample->nu;
    ResampledImage->pixdim[5]=ResampledImage->du=0;
    nifti_update_dims_from_array(ResampledImage);
    ResampledImage->datatype = NIFTI_TYPE_FLOAT32;
    ResampledImage->nbyper = sizeof(float);
    ResampledImage->data = (void *)malloc(ResampledImage->nvox * ResampledImage->nbyper);
    if(ResampledImage->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    nifti_image * Deformation = nifti_copy_nim_info(this->target->getPtr());
    Deformation->dim[0]=Deformation->ndim=5;
    Deformation->dim[1]=Deformation->nx=this->target->getPtr()->nx;
    Deformation->dim[2]=Deformation->ny=this->target->getPtr()->ny;
    Deformation->dim[3]=Deformation->nz=this->target->getPtr()->nz;
    Deformation->dim[4]=Deformation->nt=1;
    Deformation->pixdim[4]=Deformation->dt=1.0;
    Deformation->dim[5]=Deformation->nu=this->target->getPtr()->nz>1?3:2;
    Deformation->pixdim[5]=Deformation->du=1.0;
    Deformation->dim[6]=Deformation->nv=1;
    Deformation->pixdim[6]=Deformation->dv=1.0;
    Deformation->dim[7]=Deformation->nw=1;
    Deformation->pixdim[7]=Deformation->dw=1.0;
    Deformation->nvox=Deformation->nx*Deformation->ny*Deformation->nz*Deformation->nt*Deformation->nu;
    nifti_update_dims_from_array(Deformation);
    Deformation->datatype = NIFTI_TYPE_FLOAT32;
    Deformation->nbyper = sizeof(float);
    Deformation->data = (void *)calloc(Deformation->nvox, Deformation->nbyper);
    if(Deformation->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate Deformation"<< endl;
        exit(1);
    }

    reg_affine_getDeformationField(InputAffine,
                                   Deformation,
                                   false,
                                   NULL);

    // Actually resample the image given the deformation field
    reg_resampleImage(ImageToResample,
                      ResampledImage,
                      Deformation,
                      NULL,
                      typeOfInterpolation,
                      std::numeric_limits<float>::quiet_NaN());

    nifti_image_free(Deformation);

    return ResampledImage;
}


// Resample the image "ImageToResample" to the space of "TargetImage" using a deformation field "DeformationField"
nifti_image * GIF::createTransportVectorUsingDeformationField(nifti_image * VelocityGrid,nifti_image * FloatingImage,  nifti_image * TargetImage, nifti_image * DeformationField, unsigned int typeOfInterpolation)
{

    // Allocate the image being resampled
    nifti_image * WarpedFlowField=nifti_copy_nim_info(TargetImage);
    WarpedFlowField->dim[0]=WarpedFlowField->ndim=VelocityGrid->ndim;
    WarpedFlowField->dim[4]=WarpedFlowField->nt=1;
    WarpedFlowField->dim[5]=WarpedFlowField->nu=VelocityGrid->nu;
    WarpedFlowField->pixdim[5]=WarpedFlowField->du=1;
    nifti_update_dims_from_array(WarpedFlowField);
    WarpedFlowField->datatype = NIFTI_TYPE_FLOAT32;
    WarpedFlowField->nbyper = sizeof(float);
    WarpedFlowField->data = (void *)malloc(WarpedFlowField->nvox * WarpedFlowField->nbyper);
    WarpedFlowField->intent_p1=VelocityGrid->intent_p1;
    WarpedFlowField->intent_p2=VelocityGrid->intent_p2;
    if(WarpedFlowField->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Create a velocity field from the control point grid
    nifti_image * VelocityFieldImage=nifti_copy_nim_info(FloatingImage);
    VelocityFieldImage->dim[0]=VelocityFieldImage->ndim=VelocityGrid->ndim;
    VelocityFieldImage->dim[4]=VelocityFieldImage->nt=1;
    VelocityFieldImage->dim[5]=VelocityFieldImage->nu=VelocityGrid->nu;
    VelocityFieldImage->pixdim[5]=VelocityFieldImage->du=1;
    nifti_update_dims_from_array(VelocityFieldImage);
    VelocityFieldImage->datatype = NIFTI_TYPE_FLOAT32;
    VelocityFieldImage->nbyper = sizeof(float);
    VelocityFieldImage->data = (void *)calloc(VelocityFieldImage->nvox, VelocityFieldImage->nbyper);
    VelocityFieldImage->intent_p1=VelocityGrid->intent_p1;
    VelocityFieldImage->intent_p2=VelocityGrid->intent_p2;
    if(VelocityFieldImage->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate VelocityFieldImage"<< endl;
        exit(1);
    }
    reg_spline_getFlowFieldFromVelocityGrid(VelocityGrid,VelocityFieldImage);


    reg_getDisplacementFromDeformation(VelocityFieldImage);

    // Actually resample the image given the deformation field
    reg_resampleImage(VelocityFieldImage,
                      WarpedFlowField,
                      DeformationField,
                      NULL,
                      1,
                      0.f);

    // Compute the Jacobian matrices
    size_t voxelNumber = (size_t)WarpedFlowField->nx*
            WarpedFlowField->ny*WarpedFlowField->nz;
    mat33 *jacMatArray=(mat33 *)malloc(voxelNumber*sizeof(mat33));
    reg_defField_getJacobianMatrix(DeformationField,jacMatArray);

    // Reorient the gradient
    if(WarpedFlowField->datatype!=NIFTI_TYPE_FLOAT32){
        reg_print_msg_error("Unsupported datatype");
        exit(1);
    }
    float *warPtrX = static_cast<float *>(WarpedFlowField->data);
    float *warPtrY = &warPtrX[voxelNumber];
    float *warPtrZ = &warPtrY[voxelNumber];
    for(size_t i=0; i<voxelNumber; ++i){
        mat33 mat = jacMatArray[i];
        float values[3] = {warPtrX[i],warPtrY[i],warPtrZ[i]};
        mat=nifti_mat33_polar(mat);
        warPtrX[i]=
                values[0]*mat.m[0][0] +
                values[1]*mat.m[0][1] +
                values[2]*mat.m[0][2];
        warPtrY[i]=
                values[0]*mat.m[1][0] +
                values[1]*mat.m[1][1] +
                values[2]*mat.m[1][2];
        warPtrZ[i]=
                values[0]*mat.m[2][0] +
                values[1]*mat.m[2][1] +
                values[2]*mat.m[2][2];
    }
    nifti_image_free(VelocityFieldImage);
    free(jacMatArray);
    return WarpedFlowField;
}


nifti_image * GIF::createResampledImageUsingSForm(nifti_image * ImageToResample, nifti_image * TargetImage, unsigned int typeOfInterpolation)
{

    //Allocate Deformation Field Composed

    nifti_image * DeformationField =nifti_copy_nim_info(TargetImage);
    DeformationField->dim[0]=DeformationField->ndim=5;
    DeformationField->dim[4]=DeformationField->nt=1;
    DeformationField->pixdim[4]=DeformationField->dt=1.0;
    if(DeformationField->nz>1)
    {
        DeformationField->dim[5]=DeformationField->nu=3;
    }
    else
    {
        DeformationField->dim[5]=DeformationField->nu=2;
    }
    DeformationField->pixdim[5]=DeformationField->du=1.0;
    DeformationField->dim[6]=DeformationField->nv=1;
    DeformationField->pixdim[6]=DeformationField->dv=1.0;
    DeformationField->dim[7]=DeformationField->nw=1;
    DeformationField->pixdim[7]=DeformationField->dw=1.0;
    DeformationField->nvox =(size_t)DeformationField->nx*
            (size_t)DeformationField->ny*
            (size_t)DeformationField->nz*
            (size_t)DeformationField->nt*
            (size_t)DeformationField->nu;
    DeformationField->datatype = NIFTI_TYPE_FLOAT32;
    DeformationField->nbyper = sizeof(float);
    DeformationField->data = (void *)calloc(DeformationField->nvox, DeformationField->nbyper);

    // 1st - Zero the displacement field (stored in DeformationField)
    // 2nd - Covert the zero displacement image into a deformation field.
    // 3nd - The deformation field will then be equal to the sform deformation
    reg_tools_multiplyValueToImage(DeformationField,DeformationField,0.f);
    reg_getDeformationFromDisplacement(DeformationField);

    // Allocate the image being resampled
    nifti_image * ResampledImage=nifti_copy_nim_info(TargetImage);
    ResampledImage->dim[0]=ResampledImage->ndim=ImageToResample->ndim;
    ResampledImage->dim[4]=ResampledImage->nt=ImageToResample->nt;
    ResampledImage->dim[5]=ResampledImage->nu=ImageToResample->nu;
    ResampledImage->pixdim[5]=ResampledImage->du=1;
    ResampledImage->datatype = NIFTI_TYPE_FLOAT32;
    ResampledImage->nbyper = sizeof(float);
    nifti_update_dims_from_array(ResampledImage);
    ResampledImage->data = (void *)malloc(ResampledImage->nvox * ResampledImage->nbyper);
    if(ResampledImage->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    // Actually resample the image given the deformation field from the sform
    reg_resampleImage(ImageToResample,
                      ResampledImage,
                      DeformationField,
                      NULL,
                      typeOfInterpolation,
                      std::numeric_limits<float>::quiet_NaN());

    nifti_image_free(DeformationField);
    return ResampledImage;
}



nifti_image * GIF::createDeformationFieldFromGlobalTargetToDatabaseTarget(nifti_image * DeformationField, unsigned int databasenumb)
{
    //Deformation Field Composed
    nifti_image * DeformationFieldComposed = nifti_copy_nim_info(this->getDatabaseTarget(databasenumb));
    nifti_free_extensions(DeformationFieldComposed);
    DeformationFieldComposed->dim[0]=DeformationFieldComposed->ndim=5;
    DeformationFieldComposed->dim[4]=DeformationFieldComposed->nt=1;
    DeformationFieldComposed->pixdim[4]=DeformationFieldComposed->dt=1.0;
    if(DeformationFieldComposed->nz>1) DeformationFieldComposed->dim[5]=DeformationFieldComposed->nu=3;
    else DeformationFieldComposed->dim[5]=DeformationFieldComposed->nu=2;
    DeformationFieldComposed->pixdim[5]=DeformationFieldComposed->du=1.0;
    DeformationFieldComposed->dim[6]=DeformationFieldComposed->nv=1;
    DeformationFieldComposed->pixdim[6]=DeformationFieldComposed->dv=1.0;
    DeformationFieldComposed->dim[7]=DeformationFieldComposed->nw=1;
    DeformationFieldComposed->pixdim[7]=DeformationFieldComposed->dw=1.0;
    DeformationFieldComposed->nvox =(size_t)DeformationFieldComposed->nx*(size_t)DeformationFieldComposed->ny*(size_t)DeformationFieldComposed->nz*
            (size_t)DeformationFieldComposed->nt*(size_t)DeformationFieldComposed->nu;
    DeformationFieldComposed->datatype = NIFTI_TYPE_FLOAT32;
    DeformationFieldComposed->nbyper = sizeof(float);
    DeformationFieldComposed->data = (void *)calloc(DeformationFieldComposed->nvox, DeformationFieldComposed->nbyper);

    // 1st - Zero the displacement field (stored in DeformationFieldComposed)
    // 2nd - Covert the zero displacement image into a deformation field.
    // 3rd - The deformation field will then be equal to the sform deformation
    reg_tools_multiplyValueToImage(DeformationFieldComposed,DeformationFieldComposed,0.f);
    reg_getDeformationFromDisplacement(DeformationFieldComposed);

    // 4th - The previous sform deformation is composed with the given DeformationField
    // Note that composition is true
    reg_spline_getDeformationField(DeformationField,
                                   DeformationFieldComposed,
                                   NULL, // mask
                                   true, //composition
                                   true // bspline
                                   );

    return DeformationFieldComposed;
}


nifti_image * GIF::createDeformationFieldFromSubjectToGeneralTarget(unsigned int subnumb)
{

    if(this->getVerbose()>0)
        cout << "    - Getting deformation field"<< endl;

    // Allocate deformation with the same size as the target image
    nifti_image * Deformation = nifti_copy_nim_info(this->target->getPtr());
    nifti_free_extensions(Deformation);
    Deformation->dim[0]=Deformation->ndim=5;
    Deformation->dim[1]=Deformation->nx=this->target->getPtr()->nx;
    Deformation->dim[2]=Deformation->ny=this->target->getPtr()->ny;
    Deformation->dim[3]=Deformation->nz=this->target->getPtr()->nz;
    Deformation->dim[4]=Deformation->nt=1;
    Deformation->pixdim[4]=Deformation->dt=1.0;
    Deformation->dim[5]=Deformation->nu=this->target->getPtr()->nz>1?3:2;
    Deformation->pixdim[5]=Deformation->du=1.0;
    Deformation->dim[6]=Deformation->nv=1;
    Deformation->pixdim[6]=Deformation->dv=1.0;
    Deformation->dim[7]=Deformation->nw=1;
    Deformation->pixdim[7]=Deformation->dw=1.0;
    Deformation->nvox=Deformation->nx*Deformation->ny*Deformation->nz*Deformation->nt*Deformation->nu;
    nifti_update_dims_from_array(Deformation);
    Deformation->datatype = NIFTI_TYPE_FLOAT32;
    Deformation->nbyper = sizeof(float);
    Deformation->data = (void *)calloc(Deformation->nvox, Deformation->nbyper);
    if(Deformation->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate Deformation"<< endl;
        exit(1);
    }


    // Get the Deformation form the CPP
    // If it is a velocy field, use getDeformationFieldFromVelocityGrid
    if(this->cppdb->getDatasetNoDataPtr(subnumb)->intent_p1==SPLINE_VEL_GRID)
    {
        reg_spline_getDefFieldFromVelocityGrid(this->cppdb->getDatasetPtr(subnumb),Deformation,true);
    }
    // Otherwise use the getDeformationField function
    else
    {
        reg_tools_multiplyValueToImage(Deformation,Deformation,0.f);
        reg_getDeformationFromDisplacement(Deformation);
        reg_spline_getDeformationField(this->cppdb->getDatasetPtr(subnumb),Deformation,NULL,true,true);
    }

    // Clean the house
    this->cppdb->unloadDataFromDataset(subnumb);

    return Deformation;
}

void GIF::normaliseImage(nifti_image * normalise_image, nifti_image * reference_image, int * Mask){
    // copy image, sort and fill vector
    size_t img3Dsize=(normalise_image->nx*normalise_image->ny*normalise_image->nz);
    size_t countmask=0;
    size_t numbsamples=200;

    for(size_t index=0; index<img3Dsize; index++){
        if(Mask[index]==1){
            countmask++;
        }
    }

    float * referenceptr = static_cast<float *> (reference_image->data);
    float * normaliseptr = static_cast<float *> (normalise_image->data);
    float * referencesort=new float [countmask];
    float * normalisesort=new float [countmask];

    size_t countindex=0;
    for(size_t index=0; index<img3Dsize; index++){
        if(Mask[index]==1){
            referencesort[countindex]=referenceptr[index];
            normalisesort[countindex]=normaliseptr[index];
            countindex++;
        }
    }

    HeapSort(referencesort,countmask-1);
    HeapSort(normalisesort,countmask-1);

    int order=3;
    Eigen::VectorXf ImgReference(numbsamples,1);
    Eigen::MatrixXf ImgNormalise(numbsamples,order+1);
    for(size_t percentile=0; percentile<numbsamples; percentile++){
        ImgReference(percentile)=referencesort[(long)(floor(( (float)(percentile) / (float)(numbsamples) ) * (float)( countmask-1 )))];
        for(int j=0; j<(order+1); j++)
        {
            ImgNormalise(percentile,j)=pow(normalisesort[(long)(floor(( (float)(percentile) / (float)(numbsamples) ) * (float)( countmask-1 )))] , j );
        }
    }


    delete [] referencesort;
    delete [] normalisesort;

    Eigen::MatrixXf Img1TransImgNormalise=ImgNormalise.transpose()*ImgNormalise;
    Eigen::VectorXf Img1TransImgReference=ImgNormalise.transpose()*ImgReference;
    Eigen::VectorXf x;
    x=Img1TransImgNormalise.lu().solve(Img1TransImgReference); // using a LU factorization

    for(long i=0; i<(long)(img3Dsize); i++){
        float currentVal=normaliseptr[i];
        normaliseptr[i]=x(0);
        for(int j=1; j<(order+1); j++){
            normaliseptr[i]+=x(j)*pow(currentVal,j);
        }
    }

    return;
}

nifti_image * GIF::createDistanceFromSubject(unsigned int subnumb, nifti_image * Deformation)
{

    // Getting the distances if their value different from zero
    smartPointer<niftiObject> DistanceF = new niftiObject(NULL);
    if(this->weightDEF!=0.0f)
    {
        DistanceF=new niftiObject(createDistanceF(subnumb, Deformation));
    }

    smartPointer<niftiObject> DistanceL= new niftiObject(NULL);
    if((this->weightLSSD)!=0.0f)
    {
        DistanceL=new niftiObject(createDistanceL(subnumb, Deformation));
    }

    smartPointer<niftiObject> DistanceLNCC = new niftiObject(NULL);
    if((this->weightLNCC)!=0.0f)
    {
        DistanceLNCC=new niftiObject(createDistanceLNCC(subnumb, Deformation));
    }

    smartPointer<niftiObject> DistanceT1DTI = new niftiObject(NULL);
    if((this->weightT1DTI)!=0.0f)
    {
        DistanceT1DTI=new niftiObject(createDistanceT1DTI(subnumb, Deformation));
    }

    //    cout<<this->weightLNCC<<endl;
    //    nifti_set_filenames(DistanceLNCC->getPtr(), "test.nii.gz",0,0);
    //    nifti_image_write(DistanceLNCC->getPtr());
    //    exit(1);

    if(this->getVerbose()>0)
        cout << "    - Summing distances"<< endl;

    // Allocate resulting nifti_image*
    nifti_image *Distance = nifti_copy_nim_info(this->target->getPtrNoData());
    nifti_free_extensions(Distance);
    Distance->data = (void *)malloc(Distance->nvox * Distance->nbyper);
    float * DistanceDataPtr = static_cast<float *>(Distance->data);

    // Estimate W=exp(-D/temp), with D=(w_L*L+w_F*F)
    float * DistanceFptr=DistanceF->getPtr()!=NULL?static_cast<float *>(DistanceF->getPtr()->data):NULL;
    float * DistanceLptr=DistanceL->getPtr()!=NULL?static_cast<float *>(DistanceL->getPtr()->data):NULL;
    float * DistanceLNCCptr=DistanceLNCC->getPtr()!=NULL?static_cast<float *>(DistanceLNCC->getPtr()->data):NULL;
    float * DistanceT1DTIptr=DistanceT1DTI->getPtr()!=NULL?static_cast<float *>(DistanceT1DTI->getPtr()->data):NULL;
    size_t nvox=Distance->nx*Distance->ny*Distance->nz;
    size_t tpSize = ( Distance->nt>1 ? Distance->nt:1 ) * ( Distance->nu>1 ?Distance->nu:1 );

    for(long int index=0; index<nvox; index++)
    {
        DistanceDataPtr[index]=std::numeric_limits<float>::quiet_NaN();
        if(this->targetMask[index])
        {
            float DistanceTmp=0;
            for(size_t timepoint=0; timepoint<tpSize; timepoint++)
            {
            // Calculate distance for the exponential
            DistanceTmp = (DistanceLptr!=NULL) ? pow(this->weightLSSD*DistanceLptr[index+timepoint*nvox],2):0.0f;
            DistanceTmp += (DistanceFptr!=NULL) ? pow(this->weightDEF*DistanceFptr[index+timepoint*nvox],2):0.0f;
            DistanceTmp += (DistanceLNCCptr!=NULL) ? (this->weightLNCC*DistanceLNCCptr[index+timepoint*nvox]):0.0f;
            DistanceTmp += (DistanceT1DTIptr!=NULL) ? pow(this->weightT1DTI*DistanceT1DTIptr[index+timepoint*nvox],2):0.0f;
            }
            DistanceDataPtr[index]=std::fabs(DistanceTmp);
        }
    }


    //    nifti_set_filenames(Distance,string(string("lncc_")+std::to_string(subnumb)+string(".nii.gz")).c_str(),0,0);
    //    nifti_image_write(Distance);
    //    exit(0);
    // No need to delete anything, as all images are wrapped in smart pointers (except Distance, as it will be returned)
    // Return final similarity nifti image
    return Distance;
}

nifti_image * GIF::createDistanceF(unsigned int subnumb, nifti_image * Deformation)
{

    if(this->getVerbose()>0)
        cout << "    - Getting displacement field"<< endl;

    // Copy image and store it in a dataset smartpointer to manage memory. The Displacement memory clears out oautomatically.
    smartPointer<niftiObject> DisplacementNormDataset=new niftiObject(nifti_copy_nim_info(Deformation));
    nifti_image * DisplacementNorm=DisplacementNormDataset->getPtr();
    nifti_free_extensions(DisplacementNorm);
    DisplacementNorm->dim[0]=DisplacementNorm->ndim=5;
    DisplacementNorm->dim[5]=DisplacementNorm->nu=this->target->getPtr()->nz>1?3:2;
    DisplacementNorm->pixdim[5]=DisplacementNorm->du=1.0;
    nifti_update_dims_from_array(DisplacementNorm);
    DisplacementNorm->datatype = NIFTI_TYPE_FLOAT32;
    DisplacementNorm->nbyper = sizeof(float);
    DisplacementNorm->data = (void *)malloc(DisplacementNorm->nvox * DisplacementNorm->nbyper);
    if(DisplacementNorm->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate Displacement"<< endl;
        exit(1);
    }
    memcpy(DisplacementNorm->data, Deformation->data, DisplacementNorm->nvox*DisplacementNorm->nbyper);

    // Get Displacement field from deformation field for the Distance "F"
    reg_getDisplacementFromDeformation(DisplacementNorm);

    //Calculate its norm
    float * DisplacementNormDataPtr = static_cast<float *>(DisplacementNorm->data);
    unsigned int nvox3d=(DisplacementNorm->nx*DisplacementNorm->ny*DisplacementNorm->nz);
    for(unsigned int index=0; index<nvox3d; index++)
    {
        DisplacementNormDataPtr[index]*=DisplacementNormDataPtr[index];
        for(unsigned int tp=1; tp<3; tp++)
        {
            DisplacementNormDataPtr[index]+=DisplacementNormDataPtr[index+tp*nvox3d]*DisplacementNormDataPtr[index+tp*nvox3d];
        }
        DisplacementNormDataPtr[index]=sqrtf(DisplacementNormDataPtr[index]);

    }

    if(this->getVerbose()>0)
        cout << "    - Getting smooth displacement field"<< endl;

    // Copy image and store it in a dataset smartpointer to manage memory. The Displacement memory clears out oautomatically.
    // As we only want the norm, copy only the first TP
    nifti_image *DisplacementNormSmooth = nifti_copy_nim_info(DisplacementNorm);
    nifti_free_extensions(DisplacementNormSmooth);
    DisplacementNormSmooth->dim[0]=DisplacementNormSmooth->ndim=3;
    DisplacementNormSmooth->dim[4]=DisplacementNormSmooth->nt=1;
    DisplacementNormSmooth->dim[5]=DisplacementNormSmooth->nu=1;
    DisplacementNormSmooth->pixdim[4]=DisplacementNormSmooth->dt=1.0;
    DisplacementNormSmooth->pixdim[5]=DisplacementNormSmooth->du=1.0;
    nifti_update_dims_from_array(DisplacementNormSmooth);
    DisplacementNormSmooth->datatype = NIFTI_TYPE_FLOAT32;
    DisplacementNormSmooth->nbyper = sizeof(float);
    DisplacementNormSmooth->data = (void *)malloc(DisplacementNormSmooth->nvox * DisplacementNormSmooth->nbyper);
    if(DisplacementNormSmooth->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate DisplacementSmooth"<< endl;
        exit(1);
    }
    float * DisplacementNormSmoothDataPtr = static_cast<float *>(DisplacementNormSmooth->data);
    long index=0;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(DisplacementNormSmooth,DisplacementNormSmoothDataPtr,DisplacementNormDataPtr) \
    private(index)
#endif
    for(index=0; index<DisplacementNormSmooth->nvox; index++)
        DisplacementNormSmoothDataPtr[index]=DisplacementNormDataPtr[index];

    // Smooth the displacement image out
    GaussianSmoothing5D_nifti(DisplacementNormSmooth,this->targetMask,this->kernelstdDef*3.0f);

    if(this->getVerbose()>0)
        cout << "    - Getting distance F"<< endl;

    // Subtract the norms to get the high frequency ones
    // Pow(smooth "F",2). Displacement will contain the final field.

    int * targetMaskPTR=this->targetMask;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(DisplacementNormSmooth,targetMaskPTR,DisplacementNormSmoothDataPtr,DisplacementNormDataPtr) \
    private(index)
#endif
    for(index=0; index<DisplacementNormSmooth->nvox; index++)
    {
        DisplacementNormSmoothDataPtr[index]=targetMaskPTR[index]>0?powf((DisplacementNormDataPtr[index]-DisplacementNormSmoothDataPtr[index]),2):0;
    }

    GaussianSmoothing5D_nifti(DisplacementNormSmooth,this->targetMask,this->kernelstdDef);

    return DisplacementNormSmooth;
}

nifti_image * GIF::createDistanceL(unsigned int subnumb, nifti_image * Deformation)
{

    if(this->getVerbose()>0)
        cout << "    - Getting resampled image"<< endl;

    // Allocate memory and resample floating into the target space
    nifti_image *ResampledFloating = nifti_copy_nim_info(this->target->getPtr());
    nifti_free_extensions(ResampledFloating);
    ResampledFloating->datatype = NIFTI_TYPE_FLOAT32;
    ResampledFloating->nbyper = sizeof(float);
    ResampledFloating->dim[4]=ResampledFloating->nt=this->rootdb->getDatasetNoDataPtr(subnumb)->nt;
    nifti_update_dims_from_array(ResampledFloating);
    ResampledFloating->data = (void *)malloc(ResampledFloating->nvox * ResampledFloating->nbyper);
    if(ResampledFloating->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    if(this->rootdb->getDatasetNoDataPtr(subnumb)->nt!=this->target->getPtr()->nt)
    {
        cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
    }


    reg_resampleImage(this->rootdb->getDatasetPtr(subnumb),
                      ResampledFloating,
                      Deformation,
                      this->targetMask,
                      2,
                      std::numeric_limits<float>::quiet_NaN());

    this->rootdb->unloadDataFromDataset(subnumb);
    // Get image means and STD in order to Z-score them
    float * ResampledFloatingDataPtr = static_cast<float *>(ResampledFloating->data);

    size_t volSize=this->target->getPtr()->nx*this->target->getPtr()->ny*this->target->getPtr()->nz;
    size_t tpSize=max(this->target->getPtr()->nt,1);


    int * ROImask=new int [volSize]();


    for(size_t index=0; index<volSize; index++)
    {
        ROImask[index]=1;
        if(this->targetMask[index]==0)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        for(size_t timepoint=0; index<tpSize; index++)
        {
            if(isnan(ResampledFloatingDataPtr[index+timepoint*volSize]))  // or if any timepoint is nan after resampling
            {
                ROImask[index]=0;
            }
        }
    }

    float * ReferenceCopyPtr=new float [volSize*tpSize];
    memcpy((void *)ReferenceCopyPtr,this->target->getPtr()->data, volSize*tpSize*this->target->getPtr()->nbyper);

    // Normalise the images by doing a 3rd order PLS per time point
    size_t nvoxmax=0,nvox=0;
    int order=2;

    for(size_t timepoint=0; timepoint<tpSize; timepoint++)
    {
        for(size_t i=0; i<volSize; i++){
            if(ROImask[i]>0)
            {
                nvoxmax++;
            }
        }
        Eigen::MatrixXf Img1(nvoxmax+1,order+1);
        Eigen::VectorXf Img2(nvoxmax+1);
        for(size_t i=0; i<(size_t)(volSize); i++)
        {
            if(ROImask[i]>0)
            {
                Img2(nvox)=ResampledFloatingDataPtr[i+timepoint*volSize];
                nvox++;
            }
        }
        nvox=0;
        for(size_t i=0; i<(size_t)(volSize); i++){
            if(ROImask[i]>0)
            {
                for(int j=0; j<(order+1); j++){
                    Img1(nvox,j) = (j==0) ? 1 : pow(ReferenceCopyPtr[i+timepoint*volSize],j) ;
                }
                nvox++;
            }
        }
        Eigen::MatrixXf Img1TransImg1=Img1.transpose()*Img1;
        Eigen::VectorXf Img1TransImg2=Img1.transpose()*Img2;
        Eigen::VectorXf coefs;
        coefs=Img1TransImg1.lu().solve(Img1TransImg2); // using a LU factorization
        cout << coefs<<endl;
        for(size_t i=0; i<(size_t)(ResampledFloating->nvox); i++){
            float tmpval=ReferenceCopyPtr[i];
            ReferenceCopyPtr[i]=coefs(0);
            for(int j=1; j<(order+1); j++){
                ReferenceCopyPtr[i+timepoint*volSize]+=coefs(j)*pow(tmpval,j);
            }
        }
    }

    // L2 norm between the images, smooth the result and square it. The squared difference field will contain the final field "L".
    if(this->getVerbose()>0)
        cout << "    - Getting distance L"<< endl;


    for(size_t index=0; index<volSize*tpSize; index++) // calc residual per voxel
    {
        ResampledFloatingDataPtr[index]=(ResampledFloatingDataPtr[index]-ReferenceCopyPtr[index]);
    }

    //GaussianSmoothing(ResampledFloating,this->targetMask,this->kernelstd_ssd);

    for(size_t index=0; index<volSize*tpSize; index++)  // calc pow 2 per voxel
    {
        ResampledFloatingDataPtr[index]=pow(ResampledFloatingDataPtr[index],2);
    }

    GaussianSmoothing5D_nifti(ResampledFloating,ROImask,this->kernelstdSsd);

    for(size_t index=0; index<volSize; index++)
    {
        for(size_t timepoint=1; timepoint<tpSize; timepoint++)
        {
            ResampledFloatingDataPtr[index]+=ResampledFloatingDataPtr[index+timepoint*volSize]; // Collect LSSD's per time point
        }
        if(ROImask[index]==0)  // if it is outside the subject mask
        {
            ResampledFloatingDataPtr[index]=std::numeric_limits<float>::quiet_NaN();
        }
    }


    if(tpSize>1) // if the tp is larger than 1, then only return the 1st time point because there is where the LSSD is
    {
        ResampledFloating->dim[4]=ResampledFloating->nt=1;
        nifti_update_dims_from_array(ResampledFloating);
        void * tmpptr = (void *)malloc(volSize* ResampledFloating->nbyper);
        memcpy(tmpptr,ResampledFloating->data,volSize* ResampledFloating->nbyper);
        free(ResampledFloating->data);
        ResampledFloating->data = tmpptr;
    }


    delete [] ReferenceCopyPtr;
    delete [] ROImask;
    return ResampledFloating;
}


nifti_image * GIF::createDistanceLNCC(unsigned int subnumb, nifti_image * Deformation)
{

    if(this->getVerbose()>0)
        cout << "    - Getting resampled image"<< endl;

    // Allocate memory and resample floating into the target space
    nifti_image *ResampledFloating = nifti_copy_nim_info(this->target->getPtr());
    nifti_free_extensions(ResampledFloating);
    nifti_update_dims_from_array(ResampledFloating);
    ResampledFloating->datatype = NIFTI_TYPE_FLOAT32;
    ResampledFloating->dim[4]=ResampledFloating->nt=this->rootdb->getDatasetNoDataPtr(subnumb)->nt;
    nifti_update_dims_from_array(ResampledFloating);
    ResampledFloating->data = (void *)malloc(ResampledFloating->nvox * ResampledFloating->nbyper);
    if(ResampledFloating->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    if(this->rootdb->getDatasetNoDataPtr(subnumb)->nt!=this->target->getPtr()->nt)
    {
        cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
    }

    reg_resampleImage(this->rootdb->getDatasetPtr(subnumb),
                      ResampledFloating,
                      Deformation,
                      this->targetMask,
                      1,
                      std::numeric_limits<float>::quiet_NaN());



    float * ResampledFloatingDataPtr = static_cast<float *>(ResampledFloating->data);
    float * ReferencePtr=static_cast<float *>(this->target->getPtr()->data);

    size_t volSize=this->target->getPtr()->nx*this->target->getPtr()->ny*this->target->getPtr()->nz;

    size_t tpSize=min(max(ResampledFloating->nt,1),max(this->target->getPtr()->nt,1));


    // Get ROI mask
    int * ROImask=new int [volSize]();
    for(size_t index=0; index<volSize; index++)
    {
        ROImask[index]=1;
        if(this->targetMask[index]==0)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        for(size_t timepoint=0; timepoint<tpSize; timepoint++)
        {
            if(isnan(ResampledFloatingDataPtr[index+timepoint*volSize]))  // or if any timepoint is nan after resampling
            {
                ROImask[index]=0;
            }
        }
    }

    // CALC LNCC
    if(this->getVerbose()>0)
        cout << "    - Getting LNCC"<< endl;

    float * ReferenceImageMean=new float [volSize];
    float * ReferenceImageStd=new float [volSize];
    float * InputImageMean=new float [volSize];
    float * InputImageStd=new float [volSize];
    float * ReferenceInputCovar=new float [volSize];
    float * Results=new float [volSize];

    for(size_t timepoint=0; timepoint<tpSize; timepoint++)
    {

        for(size_t index=0; index<volSize; index++)
        {
            ReferenceImageMean[index]=ReferencePtr[index+timepoint*volSize];
            ReferenceImageStd[index]=ReferencePtr[index+timepoint*volSize]*ReferencePtr[index+timepoint*volSize];
            InputImageMean[index]=ResampledFloatingDataPtr[index+timepoint*volSize];
            InputImageStd[index]=ResampledFloatingDataPtr[index+timepoint*volSize]*ResampledFloatingDataPtr[index+timepoint*volSize];
            ReferenceInputCovar[index]=ReferencePtr[index+timepoint*volSize]*ResampledFloatingDataPtr[index+timepoint*volSize];
        }

        // Create a dummy nifty buffer for the gaussian blurring
        nifti_image *GaussianBuffer = nifti_copy_nim_info(this->target->getPtr());
        GaussianBuffer->dim[0]=GaussianBuffer->ndim=3;
        GaussianBuffer->dim[5]=GaussianBuffer->nu=1;
        GaussianBuffer->pixdim[5]=GaussianBuffer->du=1;
        nifti_update_dims_from_array(GaussianBuffer);
        GaussianBuffer->datatype = NIFTI_TYPE_FLOAT32;
        GaussianBuffer->nbyper = sizeof(float);



        GaussianBuffer->data=ReferenceInputCovar;
        GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdLncc);
        // Reference Img mean and std
        GaussianBuffer->data=ReferenceImageMean;
        GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdLncc);

        GaussianBuffer->data=ReferenceImageStd;
        GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdLncc);

        // Input Img mean and std
        GaussianBuffer->data=InputImageMean;
        GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdLncc);

        GaussianBuffer->data=InputImageStd;
        GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdLncc);

        GaussianBuffer->data=NULL;

        // Free the dummy
        nifti_image_free(GaussianBuffer);


        for(size_t index=0; index<volSize; index++)
        {
            if(ROImask[index]==0)
            {
                Results[index]=std::numeric_limits<float>::quiet_NaN();
            }
            else
            {
                if(timepoint==0)
                {
                    Results[index]=0;
                }
                ReferenceImageStd[index]=ReferenceImageStd[index]-ReferenceImageMean[index]*ReferenceImageMean[index];
                InputImageStd[index]=InputImageStd[index]-InputImageMean[index]*InputImageMean[index];
                ReferenceInputCovar[index]=(ReferenceInputCovar[index]-InputImageMean[index]*ReferenceImageMean[index])/(sqrtf(ReferenceImageStd[index]*InputImageStd[index]));

                Results[index]+=(1.0f-ReferenceInputCovar[index]);
            }
        }
    }
    delete [] ReferenceImageMean;
    delete [] ReferenceImageStd;
    delete [] InputImageMean;
    delete [] InputImageStd;
    delete [] ReferenceInputCovar;
    delete [] ROImask;



    ResampledFloating->dim[0]=3;
    ResampledFloating->dim[4]=ResampledFloating->nt=1;
    nifti_update_dims_from_array(ResampledFloating);
    free(ResampledFloating->data);
    ResampledFloating->data = (void *)Results;

    return ResampledFloating;
}


nifti_image * GIF::createDistanceT1DTI(unsigned int subnumb, nifti_image * Deformation)
{

    if(this->target->getPtr()->nt!=7 || this->rootdb->getDatasetNoDataPtr(subnumb)->nt!=7)
    {
        cerr << "ERROR: The T1DTI weight cannot be larger than 0 if the image is not a T1+DTI image (nt=7)"<< endl;
        exit(1);
    }

    if(this->getVerbose()>0)
        cout << "    - Getting resampled image"<< endl;

    // Allocate memory and resample floating into the target space
    nifti_image *ResampledFloating = nifti_copy_nim_info(this->target->getPtr());
    nifti_update_dims_from_array(ResampledFloating);
    ResampledFloating->datatype = NIFTI_TYPE_FLOAT32;
    ResampledFloating->nbyper = sizeof(float);
    ResampledFloating->dim[4]=ResampledFloating->nt=this->rootdb->getDatasetNoDataPtr(subnumb)->nt;
    nifti_update_dims_from_array(ResampledFloating);
    ResampledFloating->data = (void *)malloc(ResampledFloating->nvox * ResampledFloating->nbyper);
    if(ResampledFloating->data==NULL)
    {
        cerr << "ERROR:Low memory, could not allocate ResampledFloating"<< endl;
        exit(1);
    }

    if(this->rootdb->getDatasetNoDataPtr(subnumb)->nt!=this->target->getPtr()->nt)
    {
        cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
    }

    size_t volSize=this->target->getPtr()->nx*this->target->getPtr()->ny*this->target->getPtr()->nz;
    bool DTItp[7]={0,1,1,1,1,1,1};

    mat33 * JacMatrices=(mat33 *)malloc(volSize * sizeof(mat33));
    reg_defField_getJacobianMatrix(Deformation,JacMatrices);

    reg_resampleImage(this->rootdb->getDatasetPtr(subnumb),
                      ResampledFloating,
                      Deformation,
                      this->targetMask,
                      1,
                      std::numeric_limits<float>::quiet_NaN(),
                      DTItp,
                      JacMatrices);

    free(JacMatrices);
    this->rootdb->unloadDataFromDataset(subnumb);

    // Get ROI mask
    float * ResampledFloatingDataPtr = static_cast<float *>(ResampledFloating->data);
    float * ReferencePtr=static_cast<float *>(this->target->getPtr()->data);
    int * ROImask=new int [volSize]();
    for(size_t index=0; index<volSize; index++)
    {
        ROImask[index]=1;
        if(this->targetMask[index]==0)  // if it is outside the subject mask
        {
            ROImask[index]=0;
        }
        for(size_t timepoint=0; index<7; index++)
        {
            if(isnan(ResampledFloatingDataPtr[index+timepoint*volSize]))  // or if any timepoint is nan after resampling
            {
                ROImask[index]=0;
            }
        }
    }

    // CALC LNCC T1
    if(this->getVerbose()>0)
        cout << "    - Getting LNCC T1"<< endl;

    float * ReferenceImageMean=new float [volSize];
    float * ReferenceImageStd=new float [volSize];
    float * InputImageMean=new float [volSize];
    float * InputImageStd=new float [volSize];
    float * ReferenceInputCovar=new float [volSize];
    float * ResultsT1=new float [volSize];

    // GET LNCC stats
    for(size_t index=0; index<volSize; index++)
    {
        ReferenceImageMean[index]=ReferencePtr[index];
        ReferenceImageStd[index]=ReferencePtr[index]*ReferencePtr[index];
        InputImageMean[index]=ResampledFloatingDataPtr[index];
        InputImageStd[index]=ResampledFloatingDataPtr[index]*ResampledFloatingDataPtr[index];
        ReferenceInputCovar[index]=ReferencePtr[index]*ResampledFloatingDataPtr[index];
    }

    // Create a dummy nifty buffer for the gaussian blurring
    nifti_image *GaussianBuffer = nifti_copy_nim_info(this->target->getPtr());
    GaussianBuffer->dim[0]=GaussianBuffer->ndim=3;
    GaussianBuffer->dim[5]=GaussianBuffer->nu=1;
    GaussianBuffer->pixdim[5]=GaussianBuffer->du=1;
    nifti_update_dims_from_array(GaussianBuffer);
    GaussianBuffer->datatype = NIFTI_TYPE_FLOAT32;
    GaussianBuffer->nbyper = sizeof(float);

    GaussianBuffer->data=ReferenceInputCovar;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);
    // Reference Img mean and std
    GaussianBuffer->data=ReferenceImageMean;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);
    GaussianBuffer->data=ReferenceImageStd;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);
    // Input Img mean and std
    GaussianBuffer->data=InputImageMean;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);
    GaussianBuffer->data=InputImageStd;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);


    // GET LNCC values per voxel and store them in ResultsT1
    for(size_t index=0; index<volSize; index++)
    {
        if(ROImask[index]==0)
        {
            ResultsT1[index]=std::numeric_limits<float>::quiet_NaN();
        }
        else
        {
            ReferenceImageStd[index]=ReferenceImageStd[index]-ReferenceImageMean[index]*ReferenceImageMean[index];
            InputImageStd[index]=InputImageStd[index]-InputImageMean[index]*InputImageMean[index];
            ReferenceInputCovar[index]=(ReferenceInputCovar[index]-InputImageMean[index]*ReferenceImageMean[index])/(sqrtf(ReferenceImageStd[index]*InputImageStd[index]));

            ResultsT1[index]=(1.0f-ReferenceInputCovar[index]);
        }
    }

    // Cleanup T1 temps
    delete [] ReferenceImageMean;
    delete [] ReferenceImageStd;
    delete [] InputImageMean;
    delete [] InputImageStd;
    delete [] ReferenceInputCovar;

    // CALC DTI

    if(this->getVerbose()>0)
        cout << "    - Getting DTI metric"<< endl;

    // Create pointers
    float *warpedIntensityXX = &ResampledFloatingDataPtr[volSize*1];
    float *warpedIntensityXY = &ResampledFloatingDataPtr[volSize*2];
    float *warpedIntensityYY = &ResampledFloatingDataPtr[volSize*3];
    float *warpedIntensityXZ = &ResampledFloatingDataPtr[volSize*4];
    float *warpedIntensityYZ = &ResampledFloatingDataPtr[volSize*5];
    float *warpedIntensityZZ = &ResampledFloatingDataPtr[volSize*6];

    float *referenceIntensityXX = &ReferencePtr[volSize*1];
    float *referenceIntensityXY = &ReferencePtr[volSize*2];
    float *referenceIntensityYY = &ReferencePtr[volSize*3];
    float *referenceIntensityXZ = &ReferencePtr[volSize*4];
    float *referenceIntensityYZ = &ReferencePtr[volSize*5];
    float *referenceIntensityZZ = &ReferencePtr[volSize*6];

    // Create some variables to be use in the openmp loop
    long voxel;
    const double twoThirds = (2.0f/3.0f);
    float rXX, rXY, rYY, rXZ, rYZ, rZZ;
    float * ResultsDTI=new float [volSize];

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(referenceIntensityXX, referenceIntensityXY, referenceIntensityXZ, \
    referenceIntensityYY, referenceIntensityYZ, referenceIntensityZZ, \
    warpedIntensityXX,warpedIntensityXY,warpedIntensityXZ, \
    warpedIntensityYY,warpedIntensityYZ, warpedIntensityZZ, ROImask,ResultsDTI,volSize) \
    private(voxel, rXX, rXY, rYY, rXZ, rYZ, rZZ)
#endif
    for(voxel=0; voxel<volSize;++voxel){
        // Check if the current voxel belongs to the mask and the intensities are not nans
        ResultsDTI[voxel]=10;
        if(ROImask[voxel]>0 ){
            if(referenceIntensityXX[voxel]==referenceIntensityXX[voxel] &&
                    warpedIntensityXX[voxel]==warpedIntensityXX[voxel]){
                // Calculate the elementwise residual of the diffusion tensor components
                rXX = referenceIntensityXX[voxel] - warpedIntensityXX[voxel];
                rXY = referenceIntensityXY[voxel] - warpedIntensityXY[voxel];
                rYY = referenceIntensityYY[voxel] - warpedIntensityYY[voxel];
                rXZ = referenceIntensityXZ[voxel] - warpedIntensityXZ[voxel];
                rYZ = referenceIntensityYZ[voxel] - warpedIntensityYZ[voxel];
                rZZ = referenceIntensityZZ[voxel] - warpedIntensityZZ[voxel];
                ResultsDTI[voxel] = fabs(twoThirds * (reg_pow2(rXX) + reg_pow2(rYY) + reg_pow2(rZZ))
                                         + 2.0 * (reg_pow2(rXY) + reg_pow2(rXZ) + reg_pow2(rYZ))
                                         - twoThirds * (rXX*rYY+rXX*rZZ+rYY*rZZ));
            } // check if values are defined
        } // check if voxel belongs mask
    }

    // Convolve the DTI metric with a Gaussian Kernel
    GaussianBuffer->data=(void *)ResultsDTI;
    GaussianSmoothing5D_nifti(GaussianBuffer,ROImask,this->kernelstdT1DTI);
    GaussianBuffer->data=NULL;
    nifti_image_free(GaussianBuffer);

    for(voxel=0; voxel<volSize;++voxel)
    {
        // Check if the current voxel belongs to the mask and the intensities are not nans
        if(ROImask[voxel]>0)
        {
            ResultsT1[voxel]=ResultsT1[voxel]+2.0f*ResultsDTI[voxel];
        }
        else{
            ResultsT1[voxel]=std::numeric_limits<float>::quiet_NaN();
        }
    }

    // CLEANUP
    ResampledFloating->ndim=ResampledFloating->dim[0]=3;
    ResampledFloating->dim[4]=ResampledFloating->nt=1;
    ResampledFloating->dim[5]=ResampledFloating->nu=1;
    nifti_update_dims_from_array(ResampledFloating);
    free(ResampledFloating->data);
    ResampledFloating->data = (void *)ResultsT1;
    ResultsT1=NULL;

    delete [] ROImask;
    delete [] ResultsDTI;



    return ResampledFloating;
}


nifti_image * GIF::createLaplacian()
{

    if(this->targetLaplacian->getPtr()==NULL){
        if(this->getVerbose()>0)
            cout << "    - Getting Laplacian of the image"<< endl;


        const int dims[3]={(int)(this->target->getPtr()->dim[1]),
                           (int)(this->target->getPtr()->dim[2]),
                           (int)(this->target->getPtr()->dim[3])};

        long numvox=dims[0]*dims[1]*dims[2];

        float * targetPtr=static_cast<float *>(this->target->getPtr()->data);
        int nx=0;
        int ny=0;
        int nz=0;
        int index=0;

        // STATS normalisation
        double mean=0;
        double std=0;
        size_t count=0;
        int * targetmaskptr=this->targetMask;
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(targetPtr,targetmaskptr, numvox) \
    private(index) \
    reduction(+:mean) \
    reduction(+:count)
#endif
        for(index=0; index<numvox; index++)
        {
            if(targetmaskptr[index])
            {
                mean+=(double)(targetPtr[index]);
                count++;
            }
        }
        mean/=(double)(count);
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(numvox,targetmaskptr,mean,targetPtr) \
    private(index) \
    reduction(+:std)
#endif
        for(index=0; index<numvox; index++)
        {
            if(targetmaskptr[index])
            {
                std+=pow( (double)(targetPtr[index]) - mean ,2);
            }
        }
        std=(float)(sqrt((double)(std)/(double)(count)));


        float * gradientX=new float [dims[0]*dims[1]*dims[2]];
        float * gradientY=new float [dims[0]*dims[1]*dims[2]];
        float * gradientZ=new float [dims[0]*dims[1]*dims[2]];

        nifti_image *TargetTmp = nifti_copy_nim_info(this->target->getPtr());
        TargetTmp->dim[0]=TargetTmp->ndim=3;
        TargetTmp->dim[4]=TargetTmp->nt=1;
        TargetTmp->pixdim[4]=TargetTmp->dt=1;
        TargetTmp->dim[5]=TargetTmp->nu=1;
        TargetTmp->pixdim[5]=TargetTmp->du=1;
        nifti_update_dims_from_array(TargetTmp);
        TargetTmp->datatype = NIFTI_TYPE_FLOAT32;
        TargetTmp->nbyper = sizeof(float);
        float * targetNormalisedImage=new float [dims[0]*dims[1]*dims[2]];
        TargetTmp->data=(void *)targetNormalisedImage;
        nifti_set_filenames(TargetTmp, "Tmp.nii.gz", 0,0);

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(targetNormalisedImage,targetPtr,targetmaskptr,mean,std,numvox) \
    private(index) \

#endif
        for(index=0; index<numvox; index++)
        {
            if(targetmaskptr[index])
            {
                targetNormalisedImage[index]=(float)(((double)(targetPtr[index])-mean)/std);
            }
        }

        GaussianSmoothing5D_nifti(TargetTmp,this->targetMask,1.0f);

#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(dims,gradientX, gradientY, gradientZ,targetmaskptr,targetNormalisedImage) \
    private(index,nx,ny,nz)
#endif
        for(nx=0; nx<(int)dims[0]; nx++)
        {
            for(ny=0; ny<(int)dims[1]; ny++)
            {
                for(nz=0; nz<(int)dims[2]; nz++)
                {

                    index=nx+dims[0]*(ny+(dims[1]*nz));
                    if(targetmaskptr[index])
                    {
                        index=nx+dims[0]*(ny+(dims[1]*nz));
                        gradientX[index]=fabs(targetNormalisedImage[index+((nx+1)<dims[0]?1:0)]
                                -targetNormalisedImage[index-((nx-1)>=0?1:0)]);
                        gradientY[index]=fabs(targetNormalisedImage[index+((ny+1)<dims[1]?dims[0]:0)]
                                -targetNormalisedImage[index-((ny-1)>=0?dims[0]:0)]);
                        gradientZ[index]=fabs(targetNormalisedImage[index+((nz+1)<dims[2]?dims[0]*dims[1]:0)]
                                -targetNormalisedImage[index-((nz-1)>=0?dims[0]*dims[1]:0)]);
                    }
                }
            }
        }

        nifti_image_free(TargetTmp);

        float * Laplacian=new float [dims[0]*dims[1]*dims[2]*6];
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(dims,gradientX, gradientY, gradientZ,Laplacian,numvox,targetmaskptr) \
    private(index,nx,ny,nz)
#endif
        for(nx=0; nx<(int)dims[0]; nx++)
        {
            for(ny=0; ny<(int)dims[1]; ny++)
            {
                for(nz=0; nz<(int)dims[2]; nz++)
                {
                    index=nx+dims[0]*(ny+(dims[1]*nz));
                    if(targetmaskptr[index])
                    {

                        index=nx+dims[0]*(ny+(dims[1]*nz));
                        Eigen::MatrixXd m(3,3);

                        Laplacian[index+0*numvox]=gradientX[index]*gradientX[index]+0.1f;
                        Laplacian[index+3*numvox]=gradientY[index]*gradientY[index]+0.1f;
                        Laplacian[index+5*numvox]=gradientZ[index]*gradientZ[index]+0.1f;

                        Laplacian[index+1*numvox]=gradientY[index]*gradientX[index];
                        Laplacian[index+2*numvox]=gradientZ[index]*gradientX[index];
                        Laplacian[index+4*numvox]=gradientZ[index]*gradientY[index];

                        //Laplacian[index+9*numvox]=exp(-0.2*(vec[0]*(vec[0]*Laplacian[index+0*numvox]+vec[1]*Laplacian[index+1*numvox]+vec[2]*Laplacian[index+2*numvox])+
                        //                               vec[1]*(vec[0]*Laplacian[index+3*numvox]+vec[1]*Laplacian[index+4*numvox]+vec[2]*Laplacian[index+5*numvox])+
                        //                               vec[2]*(vec[0]*Laplacian[index+6*numvox]+vec[1]*Laplacian[index+7*numvox]+vec[2]*Laplacian[index+8*numvox])));


                        m(0,0)=Laplacian[index+0*numvox];
                        m(1,1)=Laplacian[index+3*numvox];
                        m(2,2)=Laplacian[index+5*numvox];
                        m(0,1)=m(1,0)=Laplacian[index+1*numvox];
                        m(2,0)=m(0,2)=Laplacian[index+2*numvox];
                        m(2,1)=m(1,2)=Laplacian[index+4*numvox];


                        Eigen::MatrixXd mInv=m.inverse();
                        Laplacian[index+0*numvox]=mInv(0,0);
                        Laplacian[index+1*numvox]=mInv(1,0);
                        Laplacian[index+2*numvox]=mInv(2,0);
                        Laplacian[index+3*numvox]=mInv(1,1);
                        Laplacian[index+4*numvox]=mInv(2,1);
                        Laplacian[index+5*numvox]=mInv(2,2);
                    }
                }
            }
        }

        delete [] gradientX;
        delete [] gradientY;
        delete [] gradientZ;

        nifti_image *LaplacianNii = nifti_copy_nim_info(this->target->getPtr());
        LaplacianNii->dim[0]=LaplacianNii->ndim=4;
        LaplacianNii->dim[4]=LaplacianNii->nt=6;
        LaplacianNii->pixdim[4]=LaplacianNii->dt=1;
        LaplacianNii->dim[5]=LaplacianNii->nu=1;
        LaplacianNii->pixdim[5]=LaplacianNii->du=1;
        nifti_update_dims_from_array(LaplacianNii);
        LaplacianNii->datatype = NIFTI_TYPE_FLOAT32;
        LaplacianNii->nbyper = sizeof(float);
        LaplacianNii->data=(void *)Laplacian;
        nifti_set_filenames(LaplacianNii, "Laplacian.nii.gz", 0,0);

        this->targetLaplacian=new niftiDynamicObject(LaplacianNii,true);
        return this->targetLaplacian->getPtr();
    }
    else{
        return this->targetLaplacian->getPtr();
    }

}

void GIF::RegisterAllSubjectsAndGetAffine()
{
    bool affines_exists=false;
#ifdef _OPENMP
    int true_max_threads=omp_get_max_threads();
    int thread_ratio=floor(double(true_max_threads)/double(this->ompj));
    int num_datasets=this->rootdb->getNumberOfDatasets();
    #pragma omp parallel for default(none) schedule(dynamic) num_threads(this->ompj) if(this->ompj>1) shared(cout, num_datasets,thread_ratio,affines_exists)
#endif
    for(size_t subjectnumb=0; subjectnumb<this->rootdb->getNumberOfDatasets(); subjectnumb++)
    {
        #ifdef _OPENMP
        omp_set_num_threads(thread_ratio);
        #endif
        if(this->cppdb->getDatasetNoDataPtr(subjectnumb)==NULL)
        {
            affines_exists=true;
            this->RegisterSubjectAffine(subjectnumb);
        }
        else
        {
            if(this->getVerbose()>0){
#ifdef _OPENMP
#pragma omp critical
#endif
                {
                    cout << "Affine Registration "<<subjectnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<endl;
                    cout << "    - Alredy registered - CPP found"<<endl;
                }
            }
        }
    }

#ifdef _OPENMP
    omp_set_num_threads(true_max_threads);
#endif

    // If all images are s-form aligned, find the average transformation in order to make it more robust.
    if(this->getSformAlignedDatabase()>0 && affines_exists)
    {
        this->AverageAffine();

        if(this->groupMask->getPtr()!=NULL){
            this->updateTargetMaskWithGroupMask();
        }
    }

    if(this->getSformAlignedDatabase()==2 && affines_exists)
    {
        for(size_t subjectnumb=0; subjectnumb<this->rootdb->getNumberOfDatasets(); subjectnumb++)
        {
            {
                this->RegisterSubjectAffineImprove(subjectnumb);
            }
        }
    }
}

void GIF::RegisterAllSubjectsAndGetCPP()
{
    int num_datasets=this->rootdb->getNumberOfDatasets();
#ifdef _OPENMP
    int true_num_threads=omp_get_max_threads();
    int thread_ratio=floor(double(true_num_threads)/double(this->ompj));

    #pragma omp parallel for default(none) schedule(dynamic) num_threads(this->ompj) if(this->ompj>1) shared(cout, num_datasets, thread_ratio)
#endif
    for(size_t subjectnumb=0; subjectnumb<num_datasets; subjectnumb++)
    {
#ifdef _OPENMP
        omp_set_num_threads(thread_ratio);
#endif
        this->RegisterSubjectNRR(subjectnumb);
        // If we're saving the cpp files to disk, we can off load them from memory and load them again when needed.
        if(this->saveTransformations==true)
        {
            this->cppdb->unloadDataFromDataset(subjectnumb);
        }
    }
#ifdef _OPENMP
    omp_set_num_threads(true_num_threads);
#endif
}

void GIF::RegisterSubjectAffine(unsigned int subnumb)
{
//#ifdef _OPENMP
//    #pragma omp critical
//    {
//        std::cout<<std::endl<<omp_get_thread_num()<<" "<<this->ompj<<" "<< omp_get_num_threads()<<std::endl;
//        flush(std::cout);
//    }
//#endif
    this->getVerbose();
    string imgname=this->rootdb->getDatasetName(subnumb).substr(this->rootdb->getDatasetName(subnumb).find_last_of(SEP)+1,this->rootdb->getDatasetName(subnumb).size()-this->rootdb->getDatasetName(subnumb).find_last_of(SEP));
    string aff_name;
    if(this->getSformAlignedDatabase()==2){
        aff_name=this->cppdb->getPath()+SEP+imgname.substr(0,min(min(imgname.find(".nii",imgname.size()-7),imgname.find(".img",imgname.size()-7)),imgname.find(".hdr",imgname.size()-7)) )+"_step1.txt";
    }
    else{
        aff_name=this->cppdb->getPath()+SEP+imgname.substr(0,min(min(imgname.find(".nii",imgname.size()-7),imgname.find(".img",imgname.size()-7)),imgname.find(".hdr",imgname.size()-7)) )+".txt";
    }

    // If the subject number is smaller than the total number of subjects
    if((size_t)subnumb<this->rootdb->getNumberOfDatasets())
    {
        // Load floating and reference images
        nifti_image * floating=this->rootdb->getDatasetNoDataPtr(subnumb);
        nifti_image * reference=this->target->getPtr();


        // Check if affine already exists
        FILE *aff;
        if(this->getAndSaveTransformations() && (bool)(aff=fopen(aff_name.c_str(), "r")))
        {
            fclose(aff);


            // if it exists, then load the affine
            reg_tool_ReadAffineFile(this->affDB[subnumb].cPTR(),
                    (char *)aff_name.c_str());

            if(this->getVerbose()>0)
            {
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
                cout << "Affine Registration "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: "<<floating->fname<<"\n -> Target: "<<reference->fname<<endl;
                cout<<"    - Alredy registered"<<endl;
#ifdef _OPENMP
    }
#endif
                if(this->getVerbose()>1)
                {
                    reg_mat44_disp(this->affDB[subnumb].cPTR(),(char *)"Affine");
                }


            }
        }
        // if it does not exist, then run Aladin
        else
        {
            time_t start,end;
            time(&start);
            int minutes;
            int seconds;

            if(this->getVerbose()>0){
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
                cout << "Affine Registration "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: "<<floating->fname<<"\n -> Target: "<<reference->fname<<endl;
                cout << "    - Affine registration started"<< endl;
#ifdef _OPENMP
    }
#endif
                }

            // Allocate mask nifti_image
            // Allocate mask nifti_image
            floating=this->rootdb->getDatasetPtr(subnumb);
            smartPointer<niftiObject> Mask=new niftiObject(this->createTargetMask());


            // Check minimum number of common time points and directly change the data to match them
            if(reference->nt!=floating->nt){
                int minTP=max(min(reference->nt,floating->nt),1);
                reference->dim[4]=reference->nt=minTP;
                floating->dim[4]=floating->nt=minTP;
                cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
            }


            // Set up and run Affine
            reg_aladin<float> *REG_AFFINE=new reg_aladin_sym<float>;
            REG_AFFINE->SetInputReference(reference);
            REG_AFFINE->SetInputFloating(floating);
            REG_AFFINE->SetInputMask(Mask->getPtr());
            REG_AFFINE->SetLevelsToPerform(2);

            if(this->getSformAlignedDatabase()==2)
            {
                REG_AFFINE->SetPerformAffine(0);
            }
            else
            {
                REG_AFFINE->SetPerformAffine(1);
            }
            // Mod SV - allow speed-up of affine registration, identical to -speed option in reg_aladin
            REG_AFFINE->SetBlockStepSize(this->regSpeed);
            if(this->inputAffine.length()>0)
            {
                if(this->getVerbose()>1)
                {
                    cout<<"    - Using the input affine as initialisation"<<endl;
                }
                REG_AFFINE->SetInputTransform((const char *) this->inputAffine.c_str());
                // If the affine is inputed, skip the rigid step.
                REG_AFFINE->SetPerformRigid(0);
            }
            else
            {
                REG_AFFINE->SetPerformRigid(1);
                REG_AFFINE->SetAlignCentre(1);
            }
            REG_AFFINE->SetVerbose(this->getVerbose()>1);
            REG_AFFINE->Run();

            // Get final affine and copy to DatabaseToTargetAffine
            mat44 * affine_tmp=REG_AFFINE->GetTransformationMatrix();

            for(int i=0; i<4; i++)
            {
                for(int j=0; j<4; j++)
                {
                    this->affDB[subnumb]->m[i][j]=affine_tmp->m[i][j];
                }
            }
            delete REG_AFFINE;

            // Printing time
            time(&end);
            minutes=(int)floorf(float(end-start)/60.0f);
            seconds=(int)(end-start - 60*minutes);
            if(this->getVerbose()>0)
            {
                if(this->getVerbose()>1)
                {
                    reg_mat44_disp(this->affDB[subnumb].cPTR(),(char *)"Affine");
                }
                cout << "    - Affine registration finished in "<<minutes<<"min "<<seconds<<"sec"<< endl;
            }

            //Writing cpp file
            if(this->getAndSaveTransformations())
            {
                if(this->getVerbose()>0)
                {
                    cout<<"    - Saving Affine to file: "<<aff_name<<endl;
                }

                WriteAffineFile(this->affDB[subnumb].cPTR(),aff_name.c_str());

            }

            this->rootdb->unloadDataFromDataset(subnumb);
        }
    }
    else
    {
        if(this->getVerbose()>0)
            cout<<"Index in RegisterSubjectAndGetCPP is too high"<<endl;
    }

    return;
}




void GIF::RegisterSubjectAffineImprove(unsigned int subnumb)
{
    this->getVerbose();
    string imgname=this->rootdb->getDatasetName(subnumb).substr(this->rootdb->getDatasetName(subnumb).find_last_of(SEP)+1,this->rootdb->getDatasetName(subnumb).size()-this->rootdb->getDatasetName(subnumb).find_last_of(SEP));
    string aff_name_step1=this->cppdb->getPath()+SEP+imgname.substr(0,min(min(imgname.find(".nii",imgname.size()-7),imgname.find(".img",imgname.size()-7)),imgname.find(".hdr",imgname.size()-7)) )+"_step1.txt";

    string aff_name=this->cppdb->getPath()+SEP+imgname.substr(0,min(min(imgname.find(".nii",imgname.size()-7),imgname.find(".img",imgname.size()-7)),imgname.find(".hdr",imgname.size()-7)) )+".txt";


    // If the subject number is smaller than the total number of subjects
    if((size_t)subnumb<this->rootdb->getNumberOfDatasets())
    {

        // Load floating and reference images
        nifti_image * floating=this->rootdb->getDatasetNoDataPtr(subnumb);
        nifti_image * reference=this->target->getPtr();


        // Check if affine already exists
        FILE *aff;
        if(this->getAndSaveTransformations() && (aff=fopen(aff_name.c_str(), "r")) != NULL)
        {
            fclose(aff);


            // if it exists, then load the affine
            reg_tool_ReadAffineFile(this->affDB[subnumb].cPTR(),
                    reference,
                    floating,
                    (char *)aff_name.c_str(),
                    false);

            if(this->getVerbose()>0)
            {
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
                cout << "Affine Registration (Step 2) "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: "<<floating->fname<<"\n -> Target: "<<reference->fname<<endl;
                cout<<"    - Alredy registered"<<endl;
#ifdef _OPENMP
    }
#endif
                if(this->getVerbose()>1)
                {
                    reg_mat44_disp(this->affDB[subnumb].cPTR(),(char *)"Affine");
                }


            }
        }
        // if it does not exist, then run Aladin
        else
        {
            time_t start,end;
            time(&start);
            int minutes;
            int seconds;

            //            nifti_set_filenames(floating,(char *)("flo.nii.gz"),0,0);
            //            nifti_image_write(floating);
            //            nifti_set_filenames(reference,(char *)("ref.nii.gz"),0,0);
            //            nifti_image_write(reference);
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
            if(this->getVerbose()>0){
                cout << "Affine Registration (Step 2) "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: "<<floating->fname<<"\n -> Target: "<<reference->fname<<endl;
                cout << "    - Affine registration started"<< endl;
            }
#ifdef _OPENMP
    }
#endif
            // Allocate mask nifti_image
            // Allocate mask nifti_image
            floating=this->rootdb->getDatasetPtr(subnumb);
            smartPointer<niftiObject> Mask=new niftiObject(this->createTargetMask());

            // Check minimum number of common time points and directly change the data to match them
            if(reference->nt!=floating->nt){
                int minTP=max(min(reference->nt,floating->nt),1);
                reference->dim[4]=reference->nt=minTP;
                floating->dim[4]=floating->nt=minTP;
                cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
            }

            // Set up and run Affine
            reg_aladin<float> *REG_AFFINE=new reg_aladin_sym<float>;
            REG_AFFINE->SetInputReference(reference);
            REG_AFFINE->SetInputFloating(floating);
            REG_AFFINE->SetInputMask(Mask->getPtr());
            REG_AFFINE->SetMaxIterations(10);
            REG_AFFINE->SetNumberOfLevels(3);
            REG_AFFINE->SetLevelsToPerform(2);
            REG_AFFINE->SetReferenceSigma(0.0f);
            REG_AFFINE->SetFloatingSigma(0.0f);
            REG_AFFINE->SetAlignCentre(1);
            REG_AFFINE->SetPerformRigid(1);
            REG_AFFINE->SetPerformAffine(1);
            REG_AFFINE->SetInputTransform((char *)(aff_name_step1.c_str()));
            REG_AFFINE->SetBlockPercentage(80);
            REG_AFFINE->SetInlierLts(80);
            REG_AFFINE->SetInterpolation(1);
            REG_AFFINE->SetVerbose(this->getVerbose()>1);
            REG_AFFINE->Run();

            // Get final affine and copy to DatabaseToTargetAffine
            mat44 * affine_tmp=REG_AFFINE->GetTransformationMatrix();

            for(int i=0; i<4; i++)
            {
                for(int j=0; j<4; j++)
                {
                    this->affDB[subnumb]->m[i][j]=affine_tmp->m[i][j];
                }
            }
            delete REG_AFFINE;

            // Printing time
            time(&end);
            minutes=(int)floorf(float(end-start)/60.0f);
            seconds=(int)(end-start - 60*minutes);
            if(this->getVerbose()>0)
            {
                if(this->getVerbose()>1)
                {
                    reg_mat44_disp(this->affDB[subnumb].cPTR(),(char *)"Affine");
                }
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
                cout << "    - Affine registration finished in "<<minutes<<"min "<<seconds<<"sec"<< endl;
#ifdef _OPENMP
    }
#endif
            }

            //Writing cpp file
            if(this->getAndSaveTransformations())
            {
                if(this->getVerbose()>0)
                {
                    cout<<"    - Saving Affine to file: "<<aff_name<<endl;
                }

                this->WriteAffineFile(this->affDB[subnumb].cPTR(),aff_name.c_str());

            }

            this->rootdb->unloadDataFromDataset(subnumb);
        }
    }
    else
    {
        if(this->getVerbose()>0)
            cout<<"Index in RegisterSubjectAndGetCPP is too high"<<endl;
    }

    return;
}

void GIF::WriteAffineFile(mat44 * mat ,
                          const char *fileName)
{
    FILE *affineFile;
    if((affineFile=fopen(fileName, "w")) != NULL){
        if(mat!=NULL)
        {
            for(int i=0; i<4; i++)
            {
                fprintf(affineFile, "%.7f %.7f %.7f %.7f\n", mat->m[i][0], mat->m[i][1], mat->m[i][2], mat->m[i][3]);
            }
            fclose(affineFile);
        }
        else
        {
            cerr<< "ERROR: Affine "<<fileName<<" is set to NULL."<<endl;
            exit(1);
        }

    }
    else
    {
        cerr<< "ERROR: Coulnd not write file "<<fileName<<". Check if the folder is writable and if disk space is available."<<endl;
        exit(1);
    }
}



void GIF::RegisterSubjectNRR(unsigned int subnumb)
{
    this->getVerbose();

//    #ifdef _OPENMP
//    #pragma omp critical
//    {
//        std::cout<<std::endl<<omp_get_thread_num()<<" "<<this->ompj<<" "<< omp_get_num_threads()<<std::endl;
//        flush(std::cout);
//    }
//    #endif
    if((size_t)subnumb<this->rootdb->getNumberOfDatasets())
    {

        nifti_image * floating=this->rootdb->getDatasetNoDataPtr(subnumb);
        nifti_image * reference=this->target->getPtr();

        if(this->cppdb->getDatasetNoDataPtr(subnumb)==NULL)
        {

            floating=this->rootdb->getDatasetPtr(subnumb);
            reference=this->target->getPtr();

            if(floating->nt!=reference->nt){
                cerr<< "ERROR: The number of time points of "<<floating->fname<< " and "<<reference->fname<<" does not match"<<endl;
                exit(1);
            }
            time_t start,end;
            time(&start);
            int minutes;
            int seconds;
#ifdef _OPENMP
    #pragma omp critical
    {
#endif
                if(verbose>0)
                cout << "NRR Registration "<<subnumb+1<<
                        "/"<<this->rootdb->getNumberOfDatasets()<<
                        "\n -> Source: "<<floating->fname<<
                        "\n -> Target: "<<reference->fname<<endl;
                time(&start);
                if(this->getVerbose()>0)
                    cout << "    - Non-Linear registration started now"<< endl;
#ifdef _OPENMP
    }
#endif
            // Allocate mask nifti_image
            smartPointer<niftiObject> Mask=new niftiObject(this->createTargetMask());



            // Check minimum number of common time points and directly change the data to match them
            if(reference->nt!=floating->nt){
                int minTP=max(min(reference->nt,floating->nt),1);
                reference->dim[4]=reference->nt=minTP;
                floating->dim[4]=floating->nt=minTP;
                cerr << "WARNING:A file in the root database does not have the same number of time points as the target image. Are you sure this is what you want?"<< endl;
            }

            // Set up and run F3D
            reg_f3d<float> * REG_F3D=new reg_f3d<float>(reference->nt, floating->nt);
            REG_F3D->SetReferenceImage(reference);
            REG_F3D->SetFloatingImage(floating);
            if(this->getVerbose()>1)
            {
                REG_F3D->PrintOutInformation();
            }
            else
            {
                REG_F3D->DoNotPrintOutInformation();
            }
            REG_F3D->SetReferenceMask(Mask->getPtr());
            REG_F3D->SetAffineTransformation(affDB[subnumb].cPTR());
            REG_F3D->SetJacobianLogWeight(this->regJacobianLogWeight);
            REG_F3D->SetBendingEnergyWeight(this->regBendingEnergyWeight);
            REG_F3D->SetMaximalIterationNumber(250);
            REG_F3D->SetLevelNumber(4);
            REG_F3D->SetLevelToPerform(3);
            REG_F3D->SetSpacing(0,-5.0f);
            if(this->nrr_metric==0){
                for(int tp=0; tp<reference->nt; tp++){ // set each TP as LNCC
                    REG_F3D->UseLNCC(tp,5.0f);
                }
            }

            REG_F3D->UseLinearInterpolation();
            REG_F3D->Run();

            // Get intermediate CPP as a dynamic nifti as it will be added to a DB
            smartPointer<niftiDynamicObject> resultCPP=new niftiDynamicObject(REG_F3D->GetControlPointPositionImage(),true);
            delete REG_F3D;

            // Unload data from floating image to save memory
            this->rootdb->unloadDataFromDataset(subnumb);

            // Printing time
            time(&end);
            minutes=(int)floorf(float(end-start)/60.0f);
            seconds=(int)(end-start - 60*minutes);
            if(this->getVerbose()>0){
                cout << "    - Non-Linear registration finished in "<<minutes<<"min "<<seconds<<"sec"<< endl;
                cout<<"    - Adding cpp to the database"<<endl;
            }

            string cpp_name=this->cppdb->getPath()+SEP+this->rootdb->getDatasetName(subnumb).substr(this->rootdb->getDatasetName(subnumb).find_last_of(SEP),this->rootdb->getDatasetName(subnumb).size()-this->rootdb->getDatasetName(subnumb).find_last_of(SEP));
            if(this->getAndSaveTransformations())
            {
                resultCPP->setFilename(cpp_name);
            }

            this->cppdb->setDataset(subnumb,resultCPP);

            //Writing cpp file
            if(this->getAndSaveTransformations())
            {
                string cpp_name=this->cppdb->getPath()+SEP+this->rootdb->getDatasetName(subnumb).substr(this->rootdb->getDatasetName(subnumb).find_last_of(SEP),this->rootdb->getDatasetName(subnumb).size()-this->rootdb->getDatasetName(subnumb).find_last_of(SEP));
                if(this->getVerbose()>0)
                {
                    cout<<"    - Saving cpp to file: "<<cpp_name<<endl;
                }
                nifti_image_write(this->cppdb->getDatasetPtr(subnumb));
            }
            //nifti_image_free(Mask);
        }
        else
        {
            if(this->getVerbose()>0)
                cout << "NRR Registration "<<subnumb+1<<"/"<<this->rootdb->getNumberOfDatasets()<<"\n -> Source: "<<floating->fname<<"\n -> Target: "<<reference->fname<<endl;

            if(this->getVerbose()>0)
                cout<<"    - Alredy registered"<<endl;
        }

        this->rootdb->unloadDataFromDataset(subnumb);
    }
    else
    {
        if(this->getVerbose()>0)
            cout<<"Index in RegisterSubjectAndGetCPP is too high"<<endl;
    }

    return;
}

void GIF::AverageAffine()
{

    string aff_name=this->cppdb->getPath()+SEP+"groupwise_affine.txt";
    FILE *aff;
    if((bool)(aff=fopen(aff_name.c_str(), "r"))){
        fclose(aff);
        // if it exists, then load the affine
        mat44 outputMatrix;
        reg_tool_ReadAffineFile(&outputMatrix,
                (char *)aff_name.c_str());

        // Copy to all individual affines
        for(size_t m=0; m<this->affDB.size(); ++m)
        {
            if(this->affDB[m].cPTR()!=NULL)
            {
                for(int i=0; i<4; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        this->affDB[m]->m[i][j]=outputMatrix.m[i][j];
                    }
                }
            }
        }
        // Save to target location
        string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                     (this->target->getFilename().find(".nii"))-
                                                                     (this->target->getFilename().find_last_of(SEP)+1));
        string aff_name2=this->resultsPath+SEP+strippedTargetFileName+"_gw_affine.txt";
        WriteAffineFile(&outputMatrix,aff_name2.c_str());
    }
    else{

        if(this->affDB.size()>2)
        {
            // Percentage outliers
            double percent=0.5;

            if(this->getVerbose()>0){
                cout << "\nFinding Group Average"<<endl;
            }
            mat44 *inputMatrices=(mat44 *)malloc(this->affDB.size() * sizeof(mat44));
            mat44 outputMatrix;

            // All the input matrices are log-ed
            for(size_t m=0; m<this->affDB.size(); ++m)
            {
                if(this->affDB[m].cPTR()!=NULL)
                {
                    inputMatrices[m] = reg_mat44_logm(this->affDB[m].cPTR());
                }
            }

            float *weight=(float *)malloc(this->affDB.size() * sizeof(float));
            float *weight2=(float *)malloc(this->affDB.size() * sizeof(float));

            for(size_t m=0; m<this->affDB.size(); ++m)
            {
                weight[m]=1;
                weight2[m]=1;
            }

            // All the exponentiated matrices are summed up into one matrix
            //temporary double are used to avoid error accumulation
            for(int iter=0; iter<10; iter++)
            {
                double tempValue[16]= {0,0,0,0,
                                       0,0,0,0,
                                       0,0,0,0,
                                       0,0,0,0
                                      };
                float sumdistance=0;
                for(size_t m=0; m<this->affDB.size(); ++m)
                {
                    if(this->affDB[m].cPTR()!=NULL)
                    {
                        tempValue[ 0]+=weight[m]*(double)inputMatrices[m].m[0][0];
                        tempValue[ 1]+=weight[m]*(double)inputMatrices[m].m[0][1];
                        tempValue[ 2]+=weight[m]*(double)inputMatrices[m].m[0][2];
                        tempValue[ 3]+=weight[m]*(double)inputMatrices[m].m[0][3];
                        tempValue[ 4]+=weight[m]*(double)inputMatrices[m].m[1][0];
                        tempValue[ 5]+=weight[m]*(double)inputMatrices[m].m[1][1];
                        tempValue[ 6]+=weight[m]*(double)inputMatrices[m].m[1][2];
                        tempValue[ 7]+=weight[m]*(double)inputMatrices[m].m[1][3];
                        tempValue[ 8]+=weight[m]*(double)inputMatrices[m].m[2][0];
                        tempValue[ 9]+=weight[m]*(double)inputMatrices[m].m[2][1];
                        tempValue[10]+=weight[m]*(double)inputMatrices[m].m[2][2];
                        tempValue[11]+=weight[m]*(double)inputMatrices[m].m[2][3];
                        tempValue[12]+=weight[m]*(double)inputMatrices[m].m[3][0];
                        tempValue[13]+=weight[m]*(double)inputMatrices[m].m[3][1];
                        tempValue[14]+=weight[m]*(double)inputMatrices[m].m[3][2];
                        tempValue[15]+=weight[m]*(double)inputMatrices[m].m[3][3];
                        sumdistance+=weight[m];
                    }
                }
                // Average matrix is computed
                tempValue[ 0] /= (double)sumdistance;
                tempValue[ 1] /= (double)sumdistance;
                tempValue[ 2] /= (double)sumdistance;
                tempValue[ 3] /= (double)sumdistance;
                tempValue[ 4] /= (double)sumdistance;
                tempValue[ 5] /= (double)sumdistance;
                tempValue[ 6] /= (double)sumdistance;
                tempValue[ 7] /= (double)sumdistance;
                tempValue[ 8] /= (double)sumdistance;
                tempValue[ 9] /= (double)sumdistance;
                tempValue[10] /= (double)sumdistance;
                tempValue[11] /= (double)sumdistance;
                tempValue[12] /= (double)sumdistance;
                tempValue[13] /= (double)sumdistance;
                tempValue[14] /= (double)sumdistance;
                tempValue[15] /= (double)sumdistance;

                // The final matrix is exponentiated
                outputMatrix.m[0][0]=(float)tempValue[ 0];
                outputMatrix.m[0][1]=(float)tempValue[ 1];
                outputMatrix.m[0][2]=(float)tempValue[ 2];
                outputMatrix.m[0][3]=(float)tempValue[ 3];
                outputMatrix.m[1][0]=(float)tempValue[ 4];
                outputMatrix.m[1][1]=(float)tempValue[ 5];
                outputMatrix.m[1][2]=(float)tempValue[ 6];
                outputMatrix.m[1][3]=(float)tempValue[ 7];
                outputMatrix.m[2][0]=(float)tempValue[ 8];
                outputMatrix.m[2][1]=(float)tempValue[ 9];
                outputMatrix.m[2][2]=(float)tempValue[10];
                outputMatrix.m[2][3]=(float)tempValue[11];
                outputMatrix.m[3][0]=(float)tempValue[12];
                outputMatrix.m[3][1]=(float)tempValue[13];
                outputMatrix.m[3][2]=(float)tempValue[14];
                outputMatrix.m[3][3]=(float)tempValue[15];
                // Free the array containing the input matrices

                for(size_t m=0; m<this->affDB.size(); ++m)
                {
                    weight[m]=10000;
                    if(this->affDB[m].cPTR()!=NULL)
                    {
                        mat44 Minus=reg_mat44_minus(&(inputMatrices[m]),&outputMatrix);

                        mat44 Minus_transpose;
                        for(int i=0; i<4; ++i)
                        {
                            for(int j=0; j<4; ++j)
                            {
                                Minus_transpose.m[i][j]=Minus.m[j][i];
                            }
                        }
                        mat44 MTM=reg_mat44_mul(&Minus_transpose,&Minus);
                        double trace=0;
                        for(size_t i=0; i<4; ++i)
                        {
                            trace+=MTM.m[i][i];
                        }
                        weight[m]=1/(sqrt(trace));
                        weight2[m]=1/(sqrt(trace));
                    }
                }

                quickSort(weight2,this->affDB.size());
                for(size_t m=0; m<this->affDB.size(); ++m)
                {
                    weight[m]=weight[m]>=weight2[(int)ceil((double)(this->affDB.size())*percent)];
                }
                outputMatrix = reg_mat44_expm(&outputMatrix);

            }



            free(inputMatrices);
            for(size_t m=0; m<this->affDB.size(); ++m)
            {
                if(this->affDB[m].cPTR()!=NULL)
                {
                    for(int i=0; i<4; i++)
                    {
                        for(int j=0; j<4; j++)
                        {
                            this->affDB[m]->m[i][j]=outputMatrix.m[i][j];
                        }
                    }
                }
            }

            if(this->getVerbose()>0)
            {
                reg_mat44_disp(&outputMatrix,(char *)"    - Matrix");
            }
            if(this->getAndSaveTransformations())
                WriteAffineFile(&outputMatrix,aff_name.c_str());

            string strippedTargetFileName=this->target->getFilename().substr((this->target->getFilename().find_last_of(SEP)+1),
                                                                         (this->target->getFilename().find(".nii"))-
                                                                         (this->target->getFilename().find_last_of(SEP)+1));
            string aff_name2=this->resultsPath+SEP+strippedTargetFileName+"_gw_affine.txt";
            WriteAffineFile(&outputMatrix,aff_name2.c_str());

        }
    }




    return;
}

void GIF::updateTargetMaskWithGroupMask(){

    if(this->getVerbose()>0)
    {
        if(this->getVerbose()>0){
            cout << "\nRessampling Group Mask"<<endl<<endl;
        }
    }
    assert(this->affDB.size() > 0);
    smartPointer<niftiObject> ResampledGroupMask = new niftiObject(createResampledImageUsingAffine(this->groupMask->getPtr(),this->target->getPtr(),this->affDB[0].cPTR(),0));

    nifti_set_filenames(ResampledGroupMask->getPtr(),(const char *)("curmask.nii.gz"),0,0);
    float * ResampledGroupMaskPtr = static_cast<float*> (ResampledGroupMask->getPtr()->data);

    for(size_t index=0; index<(this->target->getPtr()->nx*this->target->getPtr()->ny*this->target->getPtr()->nz); index++)
    {
        this->targetMask[index]=(ResampledGroupMaskPtr[index]>0)?this->targetMask[index]:0;
    }

    return;

}

#endif // _SEG_GIF_H
