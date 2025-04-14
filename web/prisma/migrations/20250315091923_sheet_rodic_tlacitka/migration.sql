/*
  Warnings:

  - Added the required column `sheetId` to the `buttons` table without a default value. This is not possible if the table is not empty.

*/
-- AlterTable
ALTER TABLE "buttons" ADD COLUMN     "sheetId" INTEGER NOT NULL;

-- AddForeignKey
ALTER TABLE "buttons" ADD CONSTRAINT "buttons_sheetId_fkey" FOREIGN KEY ("sheetId") REFERENCES "sheets"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
