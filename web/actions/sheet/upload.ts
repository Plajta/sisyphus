"use server";

import { actionClient } from "~/modules/safe-action";
import { prisma } from "~/modules/prisma";

function create2DArrayFromArray<T>(arr: T[]): T[][] {
	if (arr.length !== 9) {
		throw new Error("Input array must contain exactly 9 elements.");
	}

	const result: T[][] = [];
	const size = 3;

	for (let i = 0; i < size; i++) {
		result.push(arr.slice(i * size, (i + 1) * size));
	}

	return result;
}

export const uploadSheet = actionClient.action(async ({}) => {
	const sheets = await prisma.sheet.findMany({
		include: {
			buttons: {
				include: {
					files: true,
				},
				orderBy: {
					createdAt: "asc",
				},
			},
		},
	});

	return {
		success: true,
		data: sheets.map((sheet) => ({ ...sheet, buttons: create2DArrayFromArray(sheet.buttons) })),
	};
});
