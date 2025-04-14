-- DropForeignKey
ALTER TABLE "files" DROP CONSTRAINT "files_buttonId_fkey";

-- AlterTable
ALTER TABLE "files" ALTER COLUMN "buttonId" DROP NOT NULL;

-- AddForeignKey
ALTER TABLE "files" ADD CONSTRAINT "files_buttonId_fkey" FOREIGN KEY ("buttonId") REFERENCES "buttons"("id") ON DELETE SET NULL ON UPDATE CASCADE;
