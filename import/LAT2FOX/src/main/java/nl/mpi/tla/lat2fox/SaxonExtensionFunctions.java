package nl.mpi.tla.lat2fox;

import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.trans.XPathException;
import net.sf.saxon.value.SequenceType;
import net.sf.saxon.value.StringValue;
import net.sf.saxon.om.StructuredQName; 
import net.sf.saxon.om.Sequence;
import net.sf.saxon.expr.XPathContext;
import net.sf.saxon.lib.ExtensionFunctionCall;
import net.sf.saxon.lib.ExtensionFunctionDefinition;
import net.sf.saxon.Configuration;
import net.sf.saxon.TransformerFactoryImpl;
import net.sf.saxon.om.AxisInfo;
import net.sf.saxon.om.NodeInfo;
import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XPathCompiler;
import net.sf.saxon.s9api.XPathExecutable;
import net.sf.saxon.s9api.XPathSelector;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import net.sf.saxon.tree.NamespaceNode;
import net.sf.saxon.tree.iter.AxisIterator;

public final class SaxonExtensionFunctions {
    /**
     * Registers with Saxon 9.2+ all the extension functions 
     * <p>This method must be invoked once per TransformerFactory.
     *
     * @param factory the TransformerFactory pointing to
     * Saxon 9.2+ extension function registry 
     * (that is, a <tt>net.sf.saxon.Configuration</tt>). 
     * This object must be an instance of 
     * <tt>net.sf.saxon.TransformerFactoryImpl</tt>.
     */
    public static void registerAll(Configuration config) 
        throws Exception {
        config.registerExtensionFunction(new FileExistsDefinition());
        config.registerExtensionFunction(new CheckURLDefinition());
    }

    // -----------------------------------------------------------------------
    // sx:fileExists
    // -----------------------------------------------------------------------

    public static final class FileExistsDefinition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "fileExists");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_STRING };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_BOOLEAN;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        String path = ((StringValue) arguments[0].head()).getStringValue().replaceAll("^file://?/?","/");
                        boolean exists = (new java.io.File(path)).exists();
                        System.err.println("DBG: file["+path+"] exists?["+exists+"]");
                        seq = (new XdmAtomicValue(exists)).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: "+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }

    // -----------------------------------------------------------------------
    // sx:checkURL
    // -----------------------------------------------------------------------

    public static final class CheckURLDefinition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "checkURL");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_STRING };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_BOOLEAN;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        String url = ((StringValue) arguments[0].head()).getStringValue();
                        boolean valid = true;
                        try {
                            URL u = new URL(url);
                        } catch(MalformedURLException e) {
                            valid = false;
                            System.err.println("WRN: URL["+url+"] is invalid["+e+"]!");
                        }
                        //System.err.println("DBG: URL["+url+"] valid?["+valid+"]");
                        seq = (new XdmAtomicValue(valid)).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: "+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }
}