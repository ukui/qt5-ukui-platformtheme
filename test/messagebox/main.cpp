#include <QMessageBox>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Save               = 0x00000800,
//    SaveAll            = 0x00001000,
//    Open               = 0x00002000,
//    Yes                = 0x00004000,
//    YesToAll           = 0x00008000,
//    No                 = 0x00010000,
//    NoToAll            = 0x00020000,
//    Abort              = 0x00040000,
//    Retry              = 0x00080000,
//    Ignore             = 0x00100000,
//    Close              = 0x00200000,
//    Cancel             = 0x00400000,
//    Discard            = 0x00800000,
//    Help               = 0x01000000,
//    Apply              = 0x02000000,
//    Reset              = 0x04000000,
//    RestoreDefaults    = 0x08000000,
    QMessageBox w(QMessageBox::NoIcon, "title", "text ssssss", QMessageBox::Ok | QMessageBox::RestoreDefaults | QMessageBox::Save
                  | QMessageBox::SaveAll | QMessageBox::Open | QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No
                  | QMessageBox::NoToAll | QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore | QMessageBox::Close
                  | QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Help | QMessageBox::Apply | QMessageBox::Reset);

    w.show();
    return a.exec();
}
