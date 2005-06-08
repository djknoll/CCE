/*
 * Created on 17.05.2005
 */

/**
 * @author hbernegger
 */

public class Heap {

	private int offsetStart;

	private int actualPointer;

	public Heap(int offsetStart, int offsetEnd) {
		this.offsetStart = offsetStart;
		this.actualPointer = offsetEnd;
	}

	public int allocate(int size) throws OutOfHeapMemException {
		this.actualPointer = this.actualPointer - size;
		this.actualPointer = this.actualPointer - (this.actualPointer % 4); //global

		if (this.actualPointer < offsetStart) {
			throw new OutOfHeapMemException("allocation was not possible");
		}

		return this.actualPointer;
	}

	public void deAllocate(int adr, int size) {
		;
	}
}