#ifndef BACKUP_RESTORE_H
#define BACKUP_RESTORE_H

#include <string>

namespace BackupRestore
{
std::string CreateFullBackup();
bool RestoreFromBackup(const std::string& jsonData);
bool SaveBackupToFile(const std::string& filename);
bool LoadBackupFromFile(const std::string& filename);

} // namespace BackupRestore

#endif // BACKUP_RESTORE_H
