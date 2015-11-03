package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.util.TypeCheckHandler;

import static org.mockito.Mockito.*;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

public class FITSTest {

	private FITS fits;
	
	@Mock Context mockContext;
	@Mock SIP mockSIP;
	@Mock Resource mockResource1;
	@Mock Resource mockResource2;
	@Mock File mockFile1;
	@Mock File mockFile2;
	@Mock TypeCheckHandler mockTypeCheck;
	
	@Before
	public void setUp() {
		
		MockitoAnnotations.initMocks(this);
		
		fits = new FITS();
		
		
		//TODO use Spring to inject this instead
		fits.setTypeCheckHandler(mockTypeCheck);
	}
	
	@Test
	public void acceptFileType() throws DepositException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockTypeCheck.isFileAcceptable(mockFile1)).thenReturn(Boolean.TRUE);
		
		boolean result = fits.perform(mockContext);
		
		assertTrue("Result should be true", result);
	}

	@Test
	public void rejectFileType() throws DepositException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockTypeCheck.isFileAcceptable(mockFile1)).thenReturn(Boolean.FALSE);
		
		boolean result = fits.perform(mockContext);
		
		assertFalse("Result should be false", result);
	}
	
	//TODO Add some tests with multiple files to check
}
