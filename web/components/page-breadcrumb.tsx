"use client";

import { useParams, usePathname } from "next/navigation";
import { navItems } from "~/components/app-sidebar";
import { Breadcrumb, BreadcrumbItem, BreadcrumbList, BreadcrumbPage } from "~/components/ui/breadcrumb";

export function PageBreadcrumb() {
	const pathname = usePathname();
	const params = useParams();

	const navItem = navItems.find(
		(item) => item.url === pathname.split("/dialog")[0].replace(`/${params.sheetId}`, "")
	);

	return (
		navItem && (
			<Breadcrumb>
				<BreadcrumbList>
					<BreadcrumbItem>
						<BreadcrumbPage className="line-clamp-1">{navItem.description}</BreadcrumbPage>
					</BreadcrumbItem>
				</BreadcrumbList>
			</Breadcrumb>
		)
	);
}
