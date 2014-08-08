#include "packer.h"

XPackWriter* p_VFSWriter;
XPackReader* p_VFSReader;
using namespace std;

int main()
{
    PACK_RESULT pr;

    cout << "PAK Creator\n";

    Packer_CreateWriter(&p_VFSWriter);

    pr = p_VFSWriter->Init("PAKFILE.pak");
    PACK_ERROR_COUT(pr);

    pr = p_VFSWriter->AddFilesRecursively("..\\Data");
    PACK_ERROR_COUT(pr);

    pr = p_VFSWriter->WritePAK();
    PACK_ERROR_COUT(pr);

    Packer_ReleaseWriter();

    Packer_CreateReader(&p_VFSReader);

    pr = p_VFSReader->Init("PAKFILE.pak");
    PACK_ERROR_COUT(pr);

    cout << "Files in PAK archive: " << p_VFSReader->GetFileCount() << endl;

    Buffer ReadBuf;
    p_VFSReader->Read("Data\\config.cfg", ReadBuf);
    cout << "Read " << ReadBuf.GetSize() << " bytes of data.\n";

    /*p_VFSReader->Read("Data\\Fonts\\tahoma.ttf", 0, 0);
    p_VFSReader->Read("blabla", 0, 0);
    p_VFSReader->Read("Data\\Fonts", 0, 0);
    p_VFSReader->Read("Data\\Materials\\Material__298.cfg", 0, 0);*/

    Packer_ReleaseReader();

    system("PAUSE");

    return 0;
}