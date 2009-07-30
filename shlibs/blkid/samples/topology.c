/*
 * Copyright (C) 2009 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>

#include <blkid.h>

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

int main(int argc, char *argv[])
{
	int fd, rc;
	char *devname;
	blkid_probe pr;
	blkid_topology tp;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <device>  "
				"-- prints topology details about the device\n",
				program_invocation_short_name);
		return EXIT_FAILURE;
	}

	devname = argv[1];

	fd = open(devname, O_RDONLY);
	if (fd < 0)
		err(EXIT_FAILURE, "%s: open() failed", devname);

	pr = blkid_new_probe();
	if (!pr)
		err(EXIT_FAILURE, "faild to allocate a new libblkid probe");

	if (blkid_probe_set_device(pr, fd, 0, 0) != 0)
		err(EXIT_FAILURE, "failed to assign device to libblkid probe");

	/*
	 * NAME=value interface
	 */

	/* enable topology probing */
	blkid_probe_enable_topology(pr, TRUE);

	/* disable superblocks probing (enabled by default) */
	blkid_probe_enable_superblocks(pr, FALSE);

	rc = blkid_do_fullprobe(pr);
	if (rc == -1)
		errx(EXIT_FAILURE, "%s: blkid_do_fullprobe() failed", devname);
	else if (rc == 1)
		warnx("%s: missing topology information", devname);
	else {
		int i, nvals;

		printf("----- NAME=value interface (values: %d):\n", nvals);

		nvals = blkid_probe_numof_values(pr);

		for (i = 0; i < nvals; i++) {
			const char *name, *data;

			blkid_probe_get_value(pr, i, &name, &data, NULL);
			printf("\t%s = %s\n", name, data);
		}
	}

	/*
	 * Binary interface
	 */
	tp = blkid_probe_get_topology(pr);
	if (tp) {
		printf("----- binary interface:\n");
		printf("alignment offset : %lu\n",
				blkid_topology_get_alignment_offset(tp));
		printf("minimum io size  : %lu\n",
				blkid_topology_get_minimum_io_size(tp));
		printf("optimal io size  : %lu\n",
				blkid_topology_get_optimal_io_size(tp));
	}

	return EXIT_SUCCESS;
}
